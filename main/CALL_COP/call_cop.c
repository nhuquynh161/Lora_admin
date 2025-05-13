// #include "call_cop.h"
// #include "esp_http_client.h"
// #include "esp_log.h"

// static const char *TAG = "Call COP";


// void send_telegram_message()
// {
//     char url[512];
//     snprintf(url, sizeof(url),
//         "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s",
//         BOT_TOKEN, CHAT_ID, MESSAGE_TEXT);

//     esp_http_client_config_t config = {
//         .url = url,
//         .method = HTTP_METHOD_GET,
//     };

//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     esp_err_t err = esp_http_client_perform(client);

//     if (err == ESP_OK) {
//         ESP_LOGI(TAG, "Telegram message sent successfully. Status = %d",
//                  esp_http_client_get_status_code(client));
//     } else {
//         ESP_LOGE(TAG, "Failed to send Telegram message: %s", esp_err_to_name(err));
//     }

//     esp_http_client_cleanup(client);
// }


// """
// #include <stdio.h>
// #include <string.h>
// #include "esp_log.h"
// #include "esp_system.h"
// #include "nvs_flash.h"
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_netif.h"
// #include "esp_http_client.h"

// #define WIFI_SSID      "your_wifi_ssid"
// #define WIFI_PASS      "your_wifi_password"

// #define BOT_TOKEN      "7581280979:AAGhV_diUa7M4vIZYNQgB4Pzy8BMRvYAkcQ"  // Token của bạn
// #define CHAT_ID        "111222333"  // Chat ID từ Telegram
// #define MESSAGE_TEXT   "Canh bao: Khi gas cao!"

// static const char *TAG = "TELEGRAM";

// void send_telegram_message()
// {
//     char url[512];
//     snprintf(url, sizeof(url),
//         "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s",
//         BOT_TOKEN, CHAT_ID, MESSAGE_TEXT);

//     esp_http_client_config_t config = {
//         .url = url,
//         .method = HTTP_METHOD_GET,
//     };

//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     esp_err_t err = esp_http_client_perform(client);

//     if (err == ESP_OK) {
//         ESP_LOGI(TAG, "Telegram message sent successfully. Status = %d",
//                  esp_http_client_get_status_code(client));
//     } else {
//         ESP_LOGE(TAG, "Failed to send Telegram message: %s", esp_err_to_name(err));
//     }

//     esp_http_client_cleanup(client);
// }

// void app_main(void)
// {
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
//         ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     // Wi-Fi init
//     esp_netif_init();
//     esp_event_loop_create_default();
//     esp_netif_create_default_wifi_sta();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);
//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_SSID,
//             .password = WIFI_PASS,
//         },
//     };
//     esp_wifi_set_mode(WIFI_MODE_STA);
//     esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
//     esp_wifi_start();
//     esp_wifi_connect();

//     vTaskDelay(5000 / portTICK_PERIOD_MS);  // Chờ Wi-Fi kết nối

//     send_telegram_message();
// }
// """