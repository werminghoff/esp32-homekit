#include "homekit.hpp"

#include <sstream>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_system.h"
#include "dht22.h"

#include "wifi.hpp"
#include "hap.h"

#define TAG                "homekit"

#define ACCESSORY_NAME                "Gr0wr"
#define MANUFACTURER_NAME             "Gr0wr"
#define ACCESSORY_MODEL_NAME          "EGS v1.0"
#define DHT22_SENSOR_MODEL_NAME       "DHT22 v1.0"
#define SWITCH_MODEL_NAME             "Switch v1.0"
#define FIRMWARE_VERSION              "v1.0"
#define DHT22_DELAY 3000
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef struct {
  bool is_on;
  gpio_num_t pin;
  void* ev_handle;
  const char* name;
  const char* serial;
} SwitchConfig;

typedef struct {
  int temperature;
  gpio_num_t pin;
  void* temperature_ev_handle;
  const char* temperature_name;

  int humidity;
  void* humidity_ev_handle;
  const char* humidity_name;

  SemaphoreHandle_t ev_mutex;
  const char* serial;

} DHT22SensorConfig;


DHT22SensorConfig MakeDHT22SensorConfig(gpio_num_t pin,
                                        int humidity,
                                        const char* humidity_name,
                                        int temperature,
                                        const char* temperature_name,
                                        const char* serial,
                                        void* humidity_ev_handle = nullptr,
                                        void* temperature_ev_handle = nullptr) {

  DHT22SensorConfig config;
  config.pin = pin;
  config.humidity = humidity;
  config.temperature = temperature;
  config.temperature_name = temperature_name;
  config.humidity_name = humidity_name;
  config.serial = serial;
  config.humidity_ev_handle = humidity_ev_handle;
  config.temperature_ev_handle = temperature_ev_handle;
  return config;

}

SwitchConfig MakeSwitchConfig(bool is_on,
                              gpio_num_t pin,
                              const char* name,
                              const char* serial,
                              void* ev_handle = nullptr) {

  SwitchConfig config;
  config.is_on = is_on;
  config.pin = pin;
  config.name = name;
  config.serial = serial;
  config.ev_handle = ev_handle;
  return config;
}

static void* accessory;

static SwitchConfig switches[] = {
  MakeSwitchConfig(false, GPIO_NUM_22, "Switch 22", "80f44397ca77"),
  MakeSwitchConfig(false, GPIO_NUM_21, "Switch 21", "8e8f8b822726"),
  MakeSwitchConfig(false, GPIO_NUM_19, "Switch 19", "b4543ef85efe"),
  MakeSwitchConfig(false, GPIO_NUM_23, "Switch 23", "7410b4369e29"),

  MakeSwitchConfig(false, GPIO_NUM_18, "Switch 18", "39903be3efc5"),
  MakeSwitchConfig(false, GPIO_NUM_4,  "Switch 4",  "55623fd1e715"),
  MakeSwitchConfig(false, GPIO_NUM_17, "Switch 17", "cc992a742a66"),
  MakeSwitchConfig(false, GPIO_NUM_16, "Switch 16", "f1c769a13b26")
};

static DHT22SensorConfig dht22Sensors[] = {
  MakeDHT22SensorConfig(GPIO_NUM_32,
                        0, "Humidity",
                        0, "Temperature",
                        "deacc9bb9639")
};

static void dht22_monitoring_task(void* arg)
{
    int idx = (int)arg;

    float temperature_float = 0;
    float humidity_float = 0;

    while (1) {
      vTaskDelay( DHT22_DELAY / portTICK_RATE_MS );

      if (dht22_read(dht22Sensors[idx].pin, &temperature_float, &humidity_float) < 0) {
        vTaskDelay( DHT22_DELAY / portTICK_RATE_MS );
        taskYIELD();
        continue;
      }

      xSemaphoreTake(dht22Sensors[idx].ev_mutex, 0);

      dht22Sensors[idx].temperature = (int)(temperature_float * 100.0);
      dht22Sensors[idx].humidity = (int)(humidity_float * 100);

      if (dht22Sensors[idx].temperature_ev_handle) {
        hap_event_response(accessory, dht22Sensors[idx].temperature_ev_handle, (void*)dht22Sensors[idx].temperature);
      }

      if (dht22Sensors[idx].humidity_ev_handle) {
        hap_event_response(accessory, dht22Sensors[idx].humidity_ev_handle, (void*)dht22Sensors[idx].humidity);
      }

      xSemaphoreGive(dht22Sensors[idx].ev_mutex);
    }
}

void* identify(void* arg) {
  return (void*)true;
}

void* switch_read(void* arg) {
  int idx = (int)arg;
  return (void*)switches[idx].is_on;
}

void switch_write(void* arg, void* value, int len)
{
  int idx = (int)arg;

  int isOn = (int)value;
  if (isOn) {
    switches[idx].is_on = true;
    gpio_set_level(switches[idx].pin, 0);
  }
  else {
    switches[idx].is_on = false;
    gpio_set_level(switches[idx].pin, 1);
  }

  if (switches[idx].ev_handle) {
    hap_event_response(accessory, switches[idx].ev_handle, (void*)isOn);
  }
}

void switch_notify(void* arg, void* ev_handle, bool enable)
{
    int idx = (int)arg;
    if (enable) {
      switches[idx].ev_handle = ev_handle;
    }
    else {
      switches[idx].ev_handle = nullptr;
    }
}

void* temperature_read(void* arg) {
  int idx = (int)arg;

  xSemaphoreTake(dht22Sensors[idx].ev_mutex, 0);
  int temperature = dht22Sensors[idx].temperature;
  xSemaphoreGive(dht22Sensors[idx].ev_mutex);

  return (void*)temperature;
}

void temperature_notify(void* arg, void* ev_handle, bool enable)
{
    int idx = (int)arg;
    xSemaphoreTake(dht22Sensors[idx].ev_mutex, 0);
    if (enable) {
      dht22Sensors[idx].temperature_ev_handle = ev_handle;
    }
    else {
      dht22Sensors[idx].temperature_ev_handle = nullptr;
    }
    xSemaphoreGive(dht22Sensors[idx].ev_mutex);
}

void* humidity_read(void* arg) {
  int idx = (int)arg;

  xSemaphoreTake(dht22Sensors[idx].ev_mutex, 0);
  int humidity = dht22Sensors[idx].humidity;
  xSemaphoreGive(dht22Sensors[idx].ev_mutex);

  return (void*)humidity;
}

void humidity_notify(void* arg, void* ev_handle, bool enable)
{
    int idx = (int)arg;

    xSemaphoreTake(dht22Sensors[idx].ev_mutex, 0);
    if (enable) {
      dht22Sensors[idx].humidity_ev_handle = ev_handle;
    } else {
      dht22Sensors[idx].humidity_ev_handle = nullptr;
    }
    xSemaphoreGive(dht22Sensors[idx].ev_mutex);
}

void hap_object_init(void* arg)
{
  void* obj = hap_accessory_add(accessory);

  struct hap_characteristic cs[] = {
      {HAP_CHARACTER_IDENTIFY, (void*)true, NULL, identify, NULL, NULL}
  };
  hap_service_and_characteristics_add(accessory, obj, HAP_SERVICE_ACCESSORY_INFORMATION, cs, ARRAY_SIZE(cs));

  for(int i=0; i < ARRAY_SIZE(switches); i++) {
    struct hap_characteristic cc[] = {
        { HAP_CHARACTER_MANUFACTURER, (void*)MANUFACTURER_NAME, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_FIRMWARE_REVISION, (void*)FIRMWARE_VERSION, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_SERIAL_NUMBER, (void*)switches[i].serial, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_NAME, (void*)switches[i].name, (void*)i, NULL, NULL, NULL },
        { HAP_CHARACTER_MODEL, (void*)SWITCH_MODEL_NAME, (void*)i, NULL, NULL, NULL },
        { HAP_CHARACTER_ON, (void*)switches[i].is_on, (void*)i, switch_read, switch_write, switch_notify },
    };
    hap_service_and_characteristics_add(accessory, obj, HAP_SERVICE_SWITCHS, cc, ARRAY_SIZE(cc));
  }

  for(int i=0; i < ARRAY_SIZE(dht22Sensors); i++) {
    struct hap_characteristic cc[] = {
        { HAP_CHARACTER_MANUFACTURER, (void*)MANUFACTURER_NAME, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_FIRMWARE_REVISION, (void*)FIRMWARE_VERSION, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_SERIAL_NUMBER, (void*)dht22Sensors[i].serial, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_NAME, (void*)dht22Sensors[i].temperature_name, (void*)i, NULL, NULL, NULL },
        { HAP_CHARACTER_MODEL, (void*)DHT22_SENSOR_MODEL_NAME, (void*)i, NULL, NULL, NULL },
        { HAP_CHARACTER_CURRENT_TEMPERATURE, (void*)dht22Sensors[i].temperature, (void*)i, temperature_read, NULL, temperature_notify },
    };
    hap_service_and_characteristics_add(accessory, obj, HAP_SERVICE_TEMPERATURE_SENSOR, cc, ARRAY_SIZE(cc));
  }

  for(int i=0; i < ARRAY_SIZE(dht22Sensors); i++) {
    struct hap_characteristic cc[] = {
        { HAP_CHARACTER_MANUFACTURER, (void*)MANUFACTURER_NAME, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_FIRMWARE_REVISION, (void*)FIRMWARE_VERSION, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_SERIAL_NUMBER, (void*)dht22Sensors[i].serial, NULL, NULL, NULL, NULL },
        { HAP_CHARACTER_NAME, (void*)dht22Sensors[i].humidity_name, (void*)i, NULL, NULL, NULL },
        { HAP_CHARACTER_MODEL, (void*)DHT22_SENSOR_MODEL_NAME, (void*)i, NULL, NULL, NULL },
        { HAP_CHARACTER_CURRENT_RELATIVE_HUMIDITY, (void*)dht22Sensors[i].humidity, (void*)i, humidity_read, NULL, humidity_notify },
    };
    hap_service_and_characteristics_add(accessory, obj, HAP_SERVICE_HUMIDITY_SENSOR, cc, ARRAY_SIZE(cc));
  }
}

static bool homekit_initialized = false;
void homekit_setup() {
  if(homekit_initialized) return;
  homekit_initialized = true;

  hap_init();

  for(int i=0; i < ARRAY_SIZE(switches); i++) {
    gpio_pad_select_gpio(switches[i].pin);
    gpio_set_direction(switches[i].pin, GPIO_MODE_OUTPUT);
    switch_write((void*)i, (void*)switches[i].is_on, 0);
  }

  for(int i=0; i < ARRAY_SIZE(dht22Sensors); i++) {
    dht22Sensors[i].ev_mutex = xSemaphoreCreateBinary();
    if(dht22Sensors[i].ev_mutex == NULL) {
      ESP_LOGE(TAG, "xSemaphoreCreateBinary failed!");
      esp_restart();
    }
    xTaskCreate(&dht22_monitoring_task, dht22Sensors[i].serial, 2048, (void*)i, 5, NULL );
  }

  char accessory_id[32] = {0,};
  wifi::get_mac_address(accessory_id, false);

  hap_accessory_callback_t callback;
  callback.hap_object_init = hap_object_init;
  accessory = hap_accessory_register((char*)ACCESSORY_NAME,
                                      accessory_id,
                                      (char*)"333-44-555",
                                      (char*)MANUFACTURER_NAME,
                                      HAP_ACCESSORY_CATEGORY_OTHER,
                                      811,
                                      1,
                                      NULL,
                                      &callback);
}
