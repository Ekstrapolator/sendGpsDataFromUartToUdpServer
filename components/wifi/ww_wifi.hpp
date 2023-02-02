#pragma once

#include <esp_wifi.h>

namespace wifi
{
  enum State {nonInit, init, con, discon, gotIp};

  void initSta(wifi_mode_t wifi_mode, const std::string& ssid, const std::string& password);
}