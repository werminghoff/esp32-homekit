#include <sstream>

#include "esp_log.h"
#include "tests.hpp"

#define TAG "app_main"

extern "C" void app_main() {
  ESP_LOGI(TAG, "Running tests...");
  run_tests();
  ESP_LOGI(TAG, "Tests passed!");
}
