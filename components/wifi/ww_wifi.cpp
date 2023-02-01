#include <string>
#include <esp_err.h>
#include <cstring>
#include "ww_wifi.hpp"
#include "ww_wifiCreds.hpp"

static constexpr const char *TAG = "WIFI";
enum wifiState {init, connected, disconnected};
static void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "connecting...\n");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "connected\n");
        break;

    case IP_EVENT_STA_GOT_IP:
        //xSemaphoreGive(blockUntilGotIp);
        //task notyfie is better ?
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "DISCONNECTED EVENT\n");
        //recconncet state
        break;

    default:
        ESP_LOGI("UNSPECYFIED EVENT", "id: %d base: %s", event_id, event_base);
        break;
    }
}

void wifi::initSta(wifi_mode_t wifi_mode, const std::string& ssid, const std::string& password)
{
    esp_err_t initError;
    initError = esp_netif_init();
    initError = esp_wifi_set_storage(WIFI_STORAGE_RAM);

    //create event loope before registering
    initError = esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    initError = esp_wifi_init(&init_cfg);

    //esp register event handler register wifi related events
    initError = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL, NULL);
    //register TCP/IP related events
    initError = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL, NULL);
    //IP_EVENT_STA_GOT_IP ESP_EVENT_ANY_ID

    initError = esp_wifi_set_mode(wifi_mode);

    wifi_config_t sta_config;
    memset(&sta_config, 0, sizeof(sta_config));
    memcpy(sta_config.sta.ssid, ssid.data(), ssid.size());
    memcpy(sta_config.sta.password, password.data(), password.size());

    initError = esp_wifi_set_config(WIFI_IF_STA, &sta_config);

    initError = esp_wifi_start();
    ESP_ERROR_CHECK(initError);
    
    
}