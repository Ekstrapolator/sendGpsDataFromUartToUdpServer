#pragma once

#include <string>
#include <esp_wifi.h>

#include "ww_wifiCreds.hpp"

namespace wifi
{
  enum State {noinit, deInit, init, connected, disconnected};
  void initSta(wifi_mode_t wifi_mode, std::string ssid, std::string password);
}