#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/err.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>

#include <esp_err.h>
#include <esp_log.h>

#include "ww_udpClient.hpp"

static TaskHandle_t udpCliHandle = nullptr;
static constexpr const char TAG[] = "UDP";
struct ClientData {
  char adress[INET_ADDRSTRLEN];
  int port;
};

static void udpClientTask(void *arg) {
  uint8_t licznik = 0;
  while (1) {
    vTaskDelay(500);
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

    while (1) {
      char data_to_send[50];
      sprintf(data_to_send, "Licznik: %d\n", licznik++);
      int err = sendto(udpClientSock, data_to_send, strlen(data_to_send), 0,
                       (sockaddr *)&destAddr, sizeof(destAddr));
      if (err < 0) {
        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
        break;
      }

      vTaskDelay(100);
    }
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

    xTaskCreate(udpClientTask, "udp client task", 1024 * 3, &cData, 2,
                &udpCliHandle);
  }