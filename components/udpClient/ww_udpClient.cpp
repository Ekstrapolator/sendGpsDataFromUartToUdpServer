#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

#include <lwip/err.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>

#include <esp_err.h>
#include <esp_log.h>


#include "ww_udpClient.hpp"
#include "ww_wifi.hpp"

static TaskHandle_t udpCliHandle = nullptr;
static constexpr const char TAG[] = "UDP";
static QueueHandle_t udpLogQHe;

struct ClientData {
  char adress[INET_ADDRSTRLEN];
  int port;
};

static void udpClientTask(void *arg) {

  while (1) {
    ClientData *host = static_cast<ClientData *>(arg);

    sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = inet_addr(host->adress);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(host->port);

    int udpClientSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (udpClientSock < 0) {
      ESP_LOGE(TAG, "Unable to create socket errno: %d", errno);
      break;
    }
    ESP_LOGI(TAG, "UDP CLIENT Socket created");
    char dataToSend[512];
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    while (1) {

      if (xQueueReceive(udpLogQHe, &dataToSend, portMAX_DELAY)) {
        if(strlen(dataToSend) > 0)
        {
        int err = sendto(udpClientSock, dataToSend, strlen(dataToSend), 0,
                         (sockaddr *)&destAddr, sizeof(destAddr));
        
        ESP_LOGI(TAG, "SENDED DATA: %d", err);
        if (err < 0) {
          ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
          break;
        }
        }
      }
    }
    vTaskDelay(100);
    if (udpClientSock != -1) {
      ESP_LOGE(TAG, "Shutting down socket and restarting...");
      shutdown(udpClientSock, 0);
      close(udpClientSock);
      break;
    }
  }
  ESP_LOGW(TAG, "UDP CLIENT TASK - DELETE...");
  vTaskDelay(100);
  vTaskDelete(NULL);
}

void udp::clientStart(const char *IPV4address, const int port) {

  static ClientData cData;
  cData.port = port;
  memcpy(cData.adress, IPV4address, strlen(IPV4address) + 1);

  constexpr int qLenght = 10;
  constexpr int qSizeInBits = 512;
  udpLogQHe = xQueueCreate(qLenght, qSizeInBits);
  xTaskCreate(udpClientTask, "udp client task", 1024 * 10, &cData, 2,
              &udpCliHandle);
  wifi::registerForNotification(udpCliHandle);
}

void udp::logMessage(const char *log) {
  if (strlen(log) < 512) {
    long qAddEleStat = xQueueSend(udpLogQHe, log, 100);
    if (qAddEleStat == pdTRUE) {
      ESP_LOGI(TAG, "Data added to queue");
    }
  }
}