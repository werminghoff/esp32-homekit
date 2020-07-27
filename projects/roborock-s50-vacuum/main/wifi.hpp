#ifndef wifi_hpp
#define wifi_hpp

#include <functional>

namespace wifi {

    using connect_callback = void(void);
    using disconnect_callback = void(void);

    void init(const char* ssid, const char* password, std::function<connect_callback> on_connect, std::function<disconnect_callback> on_disconnect);

    void get_mac_address(char* mac_address, bool inverted);
    void get_raw_mac_address(uint8_t mac[6]);

    bool is_connected();
}

#endif
