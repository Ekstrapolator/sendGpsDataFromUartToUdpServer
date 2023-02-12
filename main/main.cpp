#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <iostream>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>

#include "ww_gps.hpp"
#include "ww_udpClient.hpp"
#include "ww_utulities.hpp"
#include "ww_wifi.hpp"

extern "C" {
void app_main(void);
}

void app_main(void) {
  namespace ut = utilities;

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);
  wifi::initSta(WIFI_MODE_STA, SSID, PASSWORD);
  ut::printChipInfo();
  gps::uartOneinit();
  udp::clientStart("192.168.1.7", 2020);
  while (1) {
    vTaskDelay(1000);
  }
}
