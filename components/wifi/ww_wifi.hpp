#pragma once

#include <string>
#include <esp_wifi.h>

#include "ww_wifiCreds.hpp"

namespace wifi
{
  enum State {nonInitialized, deInitialized, initialized, connected, gotIP, disconnected};
  void initSta(wifi_mode_t wifi_mode, const char* ssid, const char* password);
  State getState(void);
  void recconectSta(void);
}