#include "Smart_config.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"

#include "esp_log.h"

static const char *TAG = "Smart config WIFI";
static bool pairing = 0, Enable_LED = 0;

EventGroupHandle_t s_wifi_event_group;

void Wifi_smartconfig(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    Enable_LED = 1;

    TickType_t start_tick = xTaskGetTickCount();
    const TickType_t timeout = 60000 / portTICK_PERIOD_MS; // 30s

    while (pairing) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, 10);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if (uxBits & ESPTOUCH_DONE_BIT) {
            if (uxBits & ESPTOUCH_DONE_BIT) {
                ESP_LOGI(TAG, "SmartConfig over");
                Enable_LED = 0;
                gpio_set_level(LED, 0);
                pairing = 0;
            }
        }
        if ((xTaskGetTickCount() - start_tick) > timeout) {
            ESP_LOGW(TAG, "SmartConfig timeout (30s)");
            Enable_LED = 0;
            gpio_set_level(LED, 0);
            pairing = 0;
        }
    }
    esp_smartconfig_stop();
    vTaskDelete(NULL);
}

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        wifi_config_t wifi_conf;
        esp_wifi_get_config(WIFI_IF_STA, &wifi_conf);
        if (strlen((char *)wifi_conf.sta.ssid) != 0) {
            ESP_LOGW(TAG, "Saved SSID: %s", wifi_conf.sta.ssid);
            ESP_LOGW(TAG, "Saved PASS: %s", wifi_conf.sta.password);
            esp_wifi_connect();
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (!pairing) {
            esp_wifi_connect();
        }
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID: \"%s\"", ssid);
        ESP_LOGI(TAG, "PASSWORD: \"%s\"", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

void LED_smartconfig_blink(void * parm) {
    while (1)
    {
        if (Enable_LED){
            gpio_set_level(LED, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(LED, 0);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void Button_func(void *pvParameter)
{
    int hold_counter = 0;
    while (1) {
        if (gpio_get_level(BUTTON) == 0) {
            hold_counter += 10;
            if (hold_counter >= 1000 && !pairing) {
                ESP_LOGI(TAG, "Starting SmartConfig...");
                gpio_set_level(LED, 1);
                pairing = 1;
                esp_wifi_disconnect();
                xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, false, 1000);
                xTaskCreate(Wifi_smartconfig, "Wifi Smart config", 4096, NULL, 3, NULL);
            } else if (hold_counter < 1000 && pairing) {
                ESP_LOGI(TAG, "SmartConfig has stopped.");
                Enable_LED = 0;
                gpio_set_level(LED, 0);
                pairing = 0;
            }
        } else {
            hold_counter = 0;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void Wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}
