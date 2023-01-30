#pragma once

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <sstream>
#include <string>
#include <vector>

namespace gps {
void uartOneinit(void);
}