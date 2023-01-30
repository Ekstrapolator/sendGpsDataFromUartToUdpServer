#pragma once

// print chip info
#include <cstdio>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_system.h>

namespace utilities {
void printChipInfo(void);
}
