#include <sstream>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_event.h"
#include "tests.hpp"
#include "abstractions.hpp"
#include "functions.hpp"
#include "config.h"
#include "tests.hpp"
#include "wifi.hpp"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_system.h"

#define DEVICE_IP "10.1.1.201"
#define DEVICE_PORT 54321

#define TAG "app_main"

void init_nvs()
        {
            //Initialize NVS
            esp_err_t ret = nvs_flash_init();
            if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
            {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
            }
            ESP_ERROR_CHECK(ret);
            APP_LOG("nvs::init finished.");
        }

void test_vacuum() {
  size_t data_len = 0;
  auto data_to_send = packet_hello(data_len);

  auto response = send(data_to_send, data_len, DECRYPTED_ROBOT_TOKEN, DEVICE_IP, DEVICE_PORT);
  if(response) {

      srand(response->timestamp);

      timeval tv;//Cria a estrutura temporaria para funcao abaixo.
       tv.tv_sec = response->timestamp;//Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo!
        settimeofday(&tv, NULL);//Configura o RTC para manter a data atribuida atualizada.


      st_packet packet_findme;
      packet_findme.magic     = response->magic;
      packet_findme.unknown1  = response->unknown1;
      packet_findme.unknown2  = response->unknown2;
      packet_findme.timestamp = response->timestamp;
      packet_findme.data = packet_stop_cleaning(rand(), packet_findme.data_size);

      size_t to_send_len = 0;
      unsigned char* to_send = packet_build(packet_findme, DECRYPTED_ROBOT_TOKEN, to_send_len);

      auto second_response = send(to_send, to_send_len, DECRYPTED_ROBOT_TOKEN, DEVICE_IP, DEVICE_PORT);
      if(second_response){
          free(second_response);
      }

      free(response);
  }
}

extern "C" void app_main() {
  #if DEBUG
    //run_tests();
#endif
init_nvs();
ESP_ERROR_CHECK(esp_event_loop_create_default());


      wifi::init(WIFI_NAME,
                        WIFI_PASSWORD,
                        []() -> void {
                            printf("WiFi connected\n");
                            test_vacuum();
                        },
                        []() -> void {
                            printf("WiFi disconnected\n");
                        });



    APP_LOG("Exiting normally");
    return;
}
