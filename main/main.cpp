#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <iostream>
#include <stdio.h>

#include "ww_gps.hpp"
#include "ww_utulities.hpp"

extern "C" {
void app_main(void);
}

void app_main(void) {
  namespace ut = utilities;

      //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    ESP_ERROR_CHECK(ret);
  ut::printChipInfo();
  gps::uartOneinit();
}
