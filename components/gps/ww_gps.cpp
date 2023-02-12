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
char buff[2048];
char* newLineCharPos[20];

int parse(char* senToParse, size_t size)
{
  if (strncmp(senToParse, "$GNRMC", 6) == 0) {
    ESP_LOGI(TAG, "GNGGA frame");
    udp::logMessage(senToParse);
  }
  return 0;
}


int nmeaCheckSum(char *nmeaSentence, size_t size) {
  unsigned char calcCheckSum = 0;
  char checkSum[3];
  if (*(nmeaSentence) == '$') {
    for (int i = 1; i < size - 3; i++) {

      calcCheckSum ^= *(nmeaSentence + i);
    }
  }
  memcpy(checkSum, (nmeaSentence + (size - 3)), 2);
  checkSum[2] = '\n';
  unsigned char actualCheckSum = strtol(checkSum, NULL, 16);
  if (actualCheckSum == calcCheckSum) {
  return 0;
  }
  else{
    return -1;
  }
}



int parseGpsData(const uart_event_t* event) {

  int recivedBytes =
      uart_read_bytes(UART_NUM_1, &buff, rxBufSize, MinimumDelay);
  if (recivedBytes > 0) {
    int charCount{0};
    //read buff and save pointers to "\n" characters to newLineCharPos
    for(int i = 0; i < event->size; i++)
    {
      if (buff[i] == '$')
      {
        newLineCharPos[charCount] = &buff[i];
        charCount++;
      }
      else if(buff[i] == '\r')
      {
        buff[i] = '\n';
      }
      else if(buff[i] == '\n')
      {
        buff[i] = '\0';
      }
    }
    //iterate by all sentances in buff
    for(int i=0; i < charCount - 1; i++ )
    {
      size_t size = newLineCharPos[i+1] - newLineCharPos[i];
      if(size < 78 && (size -1) == strlen(newLineCharPos[i])) // check if sentance is less then max allowed nmea size 78 and '\0' is at correct position
      {
        //ESP_LOGI(TAG, "size: %d and lenght: %d", size, strlen(newLineCharPos[i]));
        //ESP_LOGI(TAG, "%s", newLineCharPos[i]);
        int senValid = nmeaCheckSum(newLineCharPos[i], size);
        if(senValid)
        {
          parse(newLineCharPos[i], size);
        }
      }

      //parseGNGGA(nmeaSentenceBuff);
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
        parseGpsData(&event);
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
