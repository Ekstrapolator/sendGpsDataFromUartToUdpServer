#include "driver/gpio.h"
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <sstream>
#include <string>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "ww_gps.hpp"
#include "ww_udpClient.hpp"


static QueueHandle_t uart1QHe;
static TaskHandle_t gpsHandle = nullptr;
static constexpr int RxBufSize = 1024;
static constexpr int MinimumDelay = 1;
static constexpr const char TAG[] = "GPS";
char buff[] = "static buffer !@#$%^&*\n";

int validateData(uart_event_t event) {
  std::string data(static_cast<unsigned long int>(event.size), '\0');
  std::vector<std::string> dataLines;
  dataLines.reserve(10);

  int recivedBytes =
      uart_read_bytes(UART_NUM_1, &data.at(0), RxBufSize, MinimumDelay);
  if (recivedBytes > 3) { //whye wne seted to 1 some times failing ????
    //split to lines
    std::string temp;
    for (auto it = data.begin(); it != data.end(); it++) {
      if (*it != '\n') {
        temp += *it;
      } else {
        dataLines.push_back(temp);
        temp.clear();
      }
    }
    //end
    //validet lines
    ESP_LOGI(TAG, "vector size: %d", dataLines.size());
    for(auto it = dataLines.begin(); it != dataLines.end(); it++)
    {
        if ((*it).at(0) != '$'){
            dataLines.erase(it);
        }
    }
    //end
    //send data to udp queue
    for(auto it = dataLines.begin(); it != dataLines.end(); it++)
    {
      udp::logMessage(buff);
    }
  }

  return 0;
}

static void uartReciveTask(void *arg) {
  uart_event_t event;
  while (true) {

    if (xQueueReceive(uart1QHe, (void *)&event, portMAX_DELAY)) {
      switch (event.type) {
      case UART_DATA:
        validateData(event);
        break;
      case UART_FIFO_OVF:
        break;
      case UART_BUFFER_FULL:
        break;
      case UART_BREAK:
        break;
      case UART_PARITY_ERR:
        break;
      case UART_FRAME_ERR:
        break;
      case UART_PATTERN_DET:
        break;
      default:
        ESP_LOGW(TAG, "uart event type: %d", event.type);
        break;
      }
    }
  }
}

void gps::uartOneinit(void) {

  const uart_config_t uart_config = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
      .source_clk = UART_SCLK_DEFAULT,

  };

  esp_err_t initError;
  static constexpr int uart1QueueSize = 20;
  initError = uart_driver_install(UART_NUM_1, RxBufSize * 2, 0, uart1QueueSize,
                                  &uart1QHe, 0);
  if (initError)
    ESP_LOGW(TAG, "%d", initError);
  initError = uart_param_config(UART_NUM_1, &uart_config);
  if (initError)
    ESP_LOGW(TAG, "%d", initError);
  initError = uart_set_pin(UART_NUM_1, GPIO_NUM_22, GPIO_NUM_26,
                           UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  if (initError)
    ESP_LOGW(TAG, "%d", initError);

  // run task and start reciving gps data
  xTaskCreate(uartReciveTask, "uart_recive_task", 1024 * 10, NULL, 2,
              &gpsHandle);
}
