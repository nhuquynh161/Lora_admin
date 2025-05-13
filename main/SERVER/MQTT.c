#include "MQTT.h"
#include "esp_log.h"
#include "uart_lora.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define MQTT_CONNECTED_BIT BIT0

static const char *MQTT = "MQTT";
static bool mqtt_connected = false;

esp_mqtt_client_handle_t _client;
static EventGroupHandle_t mqtt_event_group;

int Bell_status = 0;
int Bell = 0;

// char* Bell_json_handler(int dev, int bell) {
//     cJSON *json_data = cJSON_CreateObject();
//     cJSON_AddNumberToObject(json_data, "dev", dev);
//     cJSON_AddNumberToObject(json_data, _BELL, bell);
//     char *data = cJSON_PrintUnformatted(json_data);
//     cJSON_Delete(json_data);
//     return data;
// }

char* Sensor_json_handler(int dev, char *tem, char *hum, char *air, int bell) {
    cJSON *json_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_data, "dev", dev);
    cJSON_AddStringToObject(json_data, "tem", tem);
    cJSON_AddStringToObject(json_data, "hum", hum);
    cJSON_AddStringToObject(json_data, "air", air);
    cJSON_AddNumberToObject(json_data, _BELL, bell);
    char *data = cJSON_PrintUnformatted(json_data);
    cJSON_Delete(json_data);
    return data;
}

void Message_handler(void *event_data) {
    uint8_t TOPIC[100];
    uint8_t PAYLOAD[200];
    // char *mess = NULL;

    esp_mqtt_event_handle_t event = event_data;

    snprintf((char *)TOPIC, sizeof(TOPIC), "%.*s", event->topic_len, event->topic);
    snprintf((char *)PAYLOAD, sizeof(PAYLOAD), "%.*s", event->data_len, event->data);

    printf("TOPIC = %s\n", TOPIC);
    printf("PAYLOAD = %s\n", PAYLOAD);

    cJSON *contentParse = cJSON_Parse((char *)PAYLOAD);
    if (!contentParse) {
        printf("JSON parse error\n");
        return;
    }

    if (strcmp((char *)TOPIC, TOPIC_SUB_CTRL) == 0) {
        cJSON *output = cJSON_GetObjectItem(contentParse, _BELL);
        if (output == NULL) {
            printf("JSON key '%s' not found!\n", _BELL);
            cJSON_Delete(contentParse);
            return;
        }

        if (!cJSON_IsNumber(output)) {
            printf("JSON key '%s' is not a number!\n", _BELL);
            cJSON_Delete(contentParse);
            return;
        }

        if (output->valueint) Bell_status = 1;
    }

    cJSON_Delete(contentParse);
}

static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(MQTT, "Last error %s: 0x%x", message, error_code);
    }
}

void Pos_sensor(void *param) {
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)param;
    char *mess = NULL;

    while (1)
    {
        if (mqtt_connected && new_lora) {
            new_lora = 0;
            mess = Sensor_json_handler(1, tem, hum, air, Bell_status);
            if (mess != NULL) {
                esp_mqtt_client_publish(client, TOPIC_PUB_SENSOR, mess, 0, 1, 0);
                free(mess);
            }
        } else vTaskDelete(NULL);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void MQTT_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(MQTT, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    // int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            // ESP_LOGI(MQTT, "MQTT_EVENT_CONNECTED");
            // msg_id = esp_mqtt_client_publish(client, TOPIC_CONTROL_TO_MASTER, "ESP32 Connected", 0, 1, 0);
            // ESP_LOGI(MQTT, "Sent publish successful, msg_id = %d", msg_id);

            // msg_id = esp_mqtt_client_subscribe(client, TOPIC_SUB_CTRL, 0);
            esp_mqtt_client_subscribe(client, TOPIC_SUB_CTRL, 0);

            xTaskCreate(Pos_sensor, "Pos sensor", 1024 * 5, client, 5, NULL);
            // ESP_LOGI(MQTT, "Sent subscribe successful, msg_id= %d ", msg_id);

            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGI(MQTT, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            // ESP_LOGI(MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(MQTT, "MQTT_EVENT_DATA");
            Message_handler(event_data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(MQTT, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("Reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("Reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("Captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(MQTT, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
            break;
        default:
            ESP_LOGI(MQTT, "Other event id:%d", event->event_id);
            break;
    }
}

void MQTT_start(void) {
    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    _client = client;
    if (_client != NULL) {
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, MQTT_event_handler, NULL);
        while (true) {
            esp_mqtt_client_start(client);
    
            EventBits_t bits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000));
    
            if (bits & MQTT_CONNECTED_BIT) {
                ESP_LOGI("MQTT", "MQTT connected.");
                break;
            } else {
                ESP_LOGE("MQTT", "MQTT failed, trying....");
                esp_mqtt_client_stop(client);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }
    } else {
        ESP_LOGE("MQTT", "_client is NULL! Cannot start MQTT client.");
    }    
}

// void MQTT_start(void) {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .broker.address.uri = BROKER_URL,
//     };

//     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//     _client = client;
//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, MQTT_event_handler, NULL);
//     esp_mqtt_client_start(client);
// }