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
  ut::printChipInfo();
  gps::uartOneinit();
}
