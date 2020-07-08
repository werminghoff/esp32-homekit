#include <string.h>
#include "wifi.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"

#define ESP_MAXIMUM_RETRY 10

namespace wifi
{
    /* FreeRTOS event group to signal when we are connected*/
    static EventGroupHandle_t s_wifi_event_group;
    static std::function<connect_callback> on_connect;
    static std::function<disconnect_callback> on_disconnect;

    /* The event group allows multiple bits for each event, but we only care about one event
     * - are we connected to the AP with an IP? */
    const int WIFI_CONNECTED_BIT = BIT0;
    static const char *TAG = "wifi";
    static int s_retry_num = 0;

    static esp_err_t event_handler(void *ctx, system_event_t *event)
    {
        switch (event->event_id)
        {
            case SYSTEM_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

                if (wifi::on_connect != nullptr)
                    wifi::on_connect();

                s_retry_num = 0;
                xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
            {
                if (wifi::on_disconnect != nullptr)
                    wifi::on_disconnect();
                if (s_retry_num < ESP_MAXIMUM_RETRY)
                {
                    esp_wifi_connect();
                    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    s_retry_num++;
                    ESP_LOGI(TAG, "retry to connect to the AP");
                }
                else
                {
                    fflush(stdout);
                    esp_restart();
                }
                ESP_LOGI(TAG, "connect to the AP fail\n");
                break;
            }
            default:
                break;
        }
        return ESP_OK;
    }

    void init(const char *ssid, const char *password, std::function<connect_callback> on_connect, std::function<disconnect_callback> on_disconnect)
    {
        tcpip_adapter_init();

        s_wifi_event_group = xEventGroupCreate();
        wifi::on_connect = on_connect;
        wifi::on_disconnect = on_disconnect;

        ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        wifi_config_t wifi_config;
        wifi_config.sta = wifi_sta_config_t{};

        ::memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
        ::memcpy(wifi_config.sta.ssid, ssid, strlen(ssid) * sizeof(*ssid));
        ::memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));
        ::memcpy(wifi_config.sta.password, password, strlen(password) * sizeof(*password));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    bool is_connected() {
      return (xEventGroupGetBits(s_wifi_event_group) & WIFI_CONNECTED_BIT) == WIFI_CONNECTED_BIT;
    }

    void get_raw_mac_address(uint8_t mac[6]) {
      esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    }

    void get_mac_address(char* mac_address, bool inverted) {
        uint8_t mac[6];
        esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
        if(inverted) {
          sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        }
        else {
          sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
    }
} // namespace wifi
