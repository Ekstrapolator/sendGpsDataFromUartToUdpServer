#pragma once

#include <esp_wifi.h>
#include <string>

#include "ww_wifiCreds.hpp"

namespace wifi {
enum State {
  nonInitialized,
  deInitialized,
  initialized,
  connected,
  gotIP,
  disconnected
};
void initSta(wifi_mode_t wifi_mode, const char *ssid, const char *password);
State getState(void);
void registerForNotification(TaskHandle_t taskToNotifie);
} // namespace wifi