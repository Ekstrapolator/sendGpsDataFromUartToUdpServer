#include "driver/gpio.h"
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "ww_gps.hpp"
#include "ww_udpClient.hpp"


static QueueHandle_t uart1QHe;
static TaskHandle_t gpsHandle = nullptr;
static constexpr int MinimumDelay = 1;
static constexpr const char TAG[] = "GPS";

static constexpr int rxBufSize = 1024;
char buff[1024];
char* newLineCharPos[20];

int nmeaCheckSum(char *nmeaSentence, size_t size) {
  unsigned char calcCheckSum = 0;
  char checkSum[3];
  if (*(nmeaSentence + 1) == '$') {
    for (int i = 2; i < size - 4; i++) {

      calcCheckSum ^= *(nmeaSentence + i);
    }
  }
  memcpy(checkSum, (nmeaSentence + (size - 3)), 2);
  checkSum[2] = '\0';
  unsigned char actualCheckSum = strtol(checkSum, NULL, 16);
  if (actualCheckSum == calcCheckSum) {
    ESP_LOGI(TAG, "STATMENT IS VALID");
  }
  return actualCheckSum == calcCheckSum;
}

int validateData(uart_event_t* event) {

  int recivedBytes =
      uart_read_bytes(UART_NUM_1, &buff, rxBufSize, MinimumDelay);
  if (recivedBytes > 0) {
    int charCount{0};
    for(int i = 0; i < event->size; i++)
    {
      if (buff[i] == '\n')
      {
        newLineCharPos[charCount] = &buff[i];
        charCount++;
      }
    }
    //ESP_LOGI(TAG, "New line char count: %d", charCount);
    for(int i=0; i < charCount - 1; i++ )
    {
      char nmeaSentenceBuff[78];
      size_t size = newLineCharPos[i+1] - newLineCharPos[i];
      //ESP_LOGI(TAG, "size to copy: %u", size);
      memcpy(nmeaSentenceBuff, newLineCharPos[i], size);
      nmeaCheckSum(nmeaSentenceBuff, size);
      udp::logMessage(nmeaSentenceBuff);
      memset(nmeaSentenceBuff, 0, sizeof(nmeaSentenceBuff));
    }

    memset(newLineCharPos, 0, sizeof(newLineCharPos));
    memset(buff, 0, sizeof(buff));
  }

  return 0;
}

static void uartReciveTask(void *arg) {
  uart_event_t event;
  while (true) {

    if (xQueueReceive(uart1QHe, (void *)&event, portMAX_DELAY)) {
      switch (event.type) {
      case UART_DATA:
        validateData(&event);
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
  initError = uart_driver_install(UART_NUM_1, rxBufSize * 2, 0, uart1QueueSize,
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
