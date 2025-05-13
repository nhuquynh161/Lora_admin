#include "HTTP.h"
#include "esp_log.h"
#include "MQTT.h"

bool send_ = 0;

esp_err_t send_http_request(char *tem, char *hum, char *air, int bell) {
    char url[256];

    snprintf(url, sizeof(url),
             "http://msi.local/handle_guy.php?tem=%s&hum=%s&air=%s&bell=%d",
             tem, hum, air, bell);

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 200,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI("HTTP", "HTTP GET request successful, response code: %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE("HTTP", "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

esp_err_t send_database_http_request(char *tem, char *hum, char *air, int bell) {
    char url[256];
    snprintf(url, sizeof(url),
             "http://msi.local/database.php?tem=%s&hum=%s&air=%s&bell=%d",
             tem, hum, air, bell);

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 200,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI("HTTP", "HTTP GET request successful, response code: %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE("HTTP", "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}
