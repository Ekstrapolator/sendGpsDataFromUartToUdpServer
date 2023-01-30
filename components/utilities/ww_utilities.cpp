#include "ww_utulities.hpp"

namespace ut = utilities;

void ut::printChipInfo(void) {
  /* Print chip information */
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), WiFi%s%s, ", CONFIG_IDF_TARGET,
         chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);
  if (esp_flash_get_size(nullptr, &flash_size) != ESP_OK) {
    printf("Get flash size failed");
    return;
  }

  printf("%lu MB %s flash\n",
         static_cast<long unsigned int>(flash_size / (1024 * 1024)),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                       : "external");

  printf("Minimum free heap size: %ld bytes\n",
         esp_get_minimum_free_heap_size());
}
