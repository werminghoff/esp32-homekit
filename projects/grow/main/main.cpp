#include <sstream>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_event.h"
#include "config.h"
#include "wifi.hpp"
#include "homekit.hpp"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_system.h"

#define TAG "app_main"
static const gpio_num_t GPIO_WIFI_LED = GPIO_NUM_5;

void init_nvs()
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  printf("nvs::init finished.\n");
}

extern "C" void app_main() {

  init_nvs();
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  gpio_pad_select_gpio(GPIO_WIFI_LED);
  gpio_set_direction(GPIO_WIFI_LED, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_WIFI_LED, 1);

  wifi::init(WIFI_NAME,
            WIFI_PASSWORD,
            []() -> void {
                gpio_set_level(GPIO_WIFI_LED, 0);
                printf("WiFi connected\n");
                homekit_setup();
            },
            []() -> void {
                gpio_set_level(GPIO_WIFI_LED, 1);
                printf("WiFi disconnected\n");
            });
}
