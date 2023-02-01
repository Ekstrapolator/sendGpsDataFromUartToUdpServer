#pragma once

#include <esp_wifi.h>

namespace wifi
{
  void initSta(wifi_mode_t wifi_mode, const std::string& ssid, const std::string& password);
}