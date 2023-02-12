#pragma once

namespace gps {
struct GNGGA {
  int hour;
  int minute;
  int second;
  int thousand;
};
void uartOneinit(void);
} // namespace gps