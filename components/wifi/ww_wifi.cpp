#include <esp_err.h>
#include <esp_log.h>
#include <cstring>
#include <esp_netif.h>
#include <esp_event.h>
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ww_wifi.hpp"


wifi::State currentState = wifi::nonInitialized;
static constexpr const char TAG[] = "WIFI";
std::vector<TaskHandle_t> taskHe;

static void sendNotification(void)
{
    for(auto it = taskHe.begin(); it != taskHe.end(); it++)
    {
        xTaskNotifyGive(*it);
    }
}

static esp_err_t wifiReconnect()
{
    static int maximumRetry = 3, retryCount{0};
    esp_err_t initError;
    if(retryCount < maximumRetry)
    {
        initError = esp_wifi_connect();
        retryCount++;
    }
    else
    {
        esp_restart();
    }

    return initError;
}

static void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        break;

    case WIFI_EVENT_STA_CONNECTED:
        currentState = wifi::connected;
        ESP_LOGI(TAG, "CONNECTED\n");
        break;

    case IP_EVENT_STA_GOT_IP:
        currentState = wifi::gotIP;
        sendNotification();
        ESP_LOGI(TAG, "GOT IP\n");
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        if(currentState == wifi::gotIP)
        {
            wifiReconnect();
        }
        else if (currentState == wifi::connected)
        {
            wifiReconnect();
        }
        else if (currentState == wifi::initialized)
        {
            wifiReconnect();
        }
        else if (currentState == wifi::disconnected)
        {
            wifiReconnect();
        }
        ESP_LOGI(TAG, "DISCONNECTED EVENT, CURRENT STATE: %d\n", currentState);
        currentState = wifi::disconnected;
        break;
    case WIFI_EVENT_STA_BEACON_TIMEOUT:
        
        esp_wifi_disconnect();
        ESP_LOGI(TAG, "TIME OUT EVENT, CURRENT STATE: %d\n", currentState);
        break;

    default:
        ESP_LOGI("UNSPECYFIED EVENT", "id: %ld base: %s", event_id, event_base);
        break;
    }
}

void wifi::initSta(wifi_mode_t wifi_mode, const char* ssid, const char* password)
{
    esp_err_t initError;
    initError = esp_netif_init();
    if (initError)
    ESP_LOGW(TAG, "netif init: %d", initError);
    //create event loope before registering
    initError = esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    if (initError)
    ESP_LOGW(TAG, "create default: %d", initError);

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    initError = esp_wifi_init(&init_cfg);
    if (initError)
    ESP_LOGW(TAG, "init: %d", initError);
    esp_wifi_set_ps(WIFI_PS_NONE);
    initError = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (initError)
    ESP_LOGW(TAG, "set storage: %d", initError);

    //esp register event handler register wifi related events
    initError = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL, NULL);
    if (initError)
    ESP_LOGW(TAG, "event handler: %d", initError);
    //register TCP/IP related events
    initError = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL, NULL);
    if (initError)
    ESP_LOGW(TAG, "event handler register: %d", initError);
    //IP_EVENT_STA_GOT_IP ESP_EVENT_ANY_ID

    initError = esp_wifi_set_mode(wifi_mode);
    if (initError)
    ESP_LOGW(TAG, "set mode: %d", initError);

    wifi_config_t sta_config;
    memset(&sta_config, 0, sizeof(sta_config));
    memcpy(sta_config.sta.ssid, ssid, strlen(ssid) + 1);
    memcpy(sta_config.sta.password, password, strlen(password) + 1);

    initError = esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    if (initError)
    ESP_LOGW(TAG, "set config: %d", initError);
    initError = esp_wifi_start();
    if (initError)
    ESP_LOGW(TAG, "start: %d", initError);
    else
    currentState = wifi::initialized;
}

void wifi::registerForNotification(TaskHandle_t taskToNotifie)
{
    taskHe.push_back(taskToNotifie);
}

wifi::State wifi::getState()
{
    return currentState;
}
