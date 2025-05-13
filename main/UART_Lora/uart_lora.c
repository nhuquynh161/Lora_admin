#include "uart_lora.h"
#include "esp_log.h"
#include "MQTT.h"
#include "HTTP.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "cJSON.h"

static const char *TAG = "UART Events";
static const char *JS = "JSON log";
static QueueHandle_t uart0_queue;
static bool done = 0;
bool new_lora = 0;

char tem[10] = "0.0";
char hum[10] = "0.0";
char air[10] = "0.0";

int64_t last_send_time = 0;
void periodic_http_sender(char *tem, char *hum, char *air, int bell) {
    int64_t now = esp_timer_get_time();

    if (now - last_send_time >= 60000000) { // 10 minutes
        send_database_http_request(tem, hum, air, bell);
        last_send_time = now;
    }
}

void Handle_data_receive(char *data) {
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        ESP_LOGW(JS, "Parse JSON failed.");
        return;
    }

    cJSON *item = json->child;
    uint8_t mode = 0;
    char *mess = NULL;
    while (item) {
        if (strcmp(item->string, "dev") == 0) {
            mode = 1;
        }
        item = item->next;
    }

    if (mode == 0) {
        cJSON_Delete(json);
        return;
    } else if (mode == 1) {
        new_lora = 1;
        cJSON *Jtem = cJSON_GetObjectItem(json, "tem");
        cJSON *Jhum = cJSON_GetObjectItem(json, "hum");
        cJSON *Jair = cJSON_GetObjectItem(json, "air");
        cJSON *Jbell = cJSON_GetObjectItem(json, _BELL);
        if (Jtem && cJSON_IsString(Jtem)) snprintf(tem, sizeof(tem), "%s", Jtem->valuestring);
        if (Jhum && cJSON_IsString(Jhum)) snprintf(hum, sizeof(hum), "%s", Jhum->valuestring);
        if (Jair && cJSON_IsString(Jair)) snprintf(air, sizeof(air), "%s", Jair->valuestring);
        if (Jbell && cJSON_IsNumber(Jbell)) Bell = Jbell->valueint;

        snprintf(tem, sizeof(tem), "%s", Jtem->valuestring);
        snprintf(hum, sizeof(hum), "%s", Jhum->valuestring);
        snprintf(air, sizeof(air), "%s", Jair->valuestring);
        Bell = Jbell->valueint;
        mess = Sensor_json_handler(1, tem, hum, air, Bell);
        esp_mqtt_client_publish(_client, TOPIC_PUB_SENSOR, mess, 0, 1, 0);

        // Send database PHP server
        periodic_http_sender(Jtem->valuestring, Jhum->valuestring, Jair->valuestring, Bell);

        // Send burning PHP server
        if (!send_) {
            if (Bell) {
                send_ = 1;
                send_http_request(Jtem->valuestring, Jhum->valuestring, Jair->valuestring, Bell);
            }
        } else {
            if (!Bell) {
                send_ = 0;
            }
        }
    }

    if (mess) free(mess);
    cJSON_Delete(json);
}

static void uart_event_task(void *pvParameters) {
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for (;;) {
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            switch (event.type) {
            case UART_DATA:
                uart_read_bytes(LORA_UART_NUM, dtmp, event.size, portMAX_DELAY);
                // ESP_LOGW(TAG, "Got len: %d", event.size);
                // ESP_LOGI(TAG, "Data received: %s", (char *)dtmp);
                Handle_data_receive((char *)dtmp);
                done = 0;
                break;
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(LORA_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(LORA_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(LORA_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(LORA_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1) {
                    uart_flush_input(LORA_UART_NUM);
                } else {
                    uart_read_bytes(LORA_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(LORA_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void UART_event_config(void) {
    gpio_init();
    esp_log_level_set(TAG, ESP_LOG_INFO);

    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(LORA_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(LORA_UART_NUM, &uart_config);

    esp_log_level_set(TAG, ESP_LOG_INFO);
    uart_set_pin(LORA_UART_NUM, TXD, RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_enable_pattern_det_baud_intr(LORA_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
    uart_pattern_queue_reset(LORA_UART_NUM, 20);

    xTaskCreate(uart_event_task, "uart_event_task", 1024 * 4, NULL, 12, NULL);
}

void Busy_check(void) {
    while (gpio_get_level(AUX) == 0) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        // ESP_LOGI(TAG, "I'm busy...");
    }
    vTaskDelay(2 / portTICK_PERIOD_MS);
}

void Set_mode(uint8_t mode) {
    Busy_check();
    switch (mode) {
        case 0:
        case 1:
            gpio_set_level(M0, mode);
            gpio_set_level(M1, mode);
            break;
        case 2:
            gpio_set_level(M0, 1);
            gpio_set_level(M1, 0);
            break;
        default:
            gpio_set_level(M0, 0);
            gpio_set_level(M1, 1);
            break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void Lora_get_info(void) {
    Set_mode(CONFIG_MODE);

    uint8_t send[3] = {0xC1, 0xC1, 0xC1};
    uint8_t receive[6];

    Busy_check();
    uart_write_bytes(LORA_UART_NUM, (const char *) send, 3);
    Busy_check();
    uart_read_bytes(LORA_UART_NUM, receive, 6, 20);

    for (uint8_t i = 0; i < 6; i++) {
        printf("0x%02X ", receive[i]);
    }
    printf("\n");

    vTaskDelay(50 / portTICK_PERIOD_MS);
}

/*
    address = 0x0000-0xFFFF
    channel = 0-31 (with v1.0) 0-255 (with v1.2)
*/
void Lora_set_address(uint16_t address, uint8_t channel) {
    uint8_t send[6] = {0xC0, (address >> 8), (address & 0xFF), 0x1A, (channel & 0xFF), 0x44};
    Busy_check();

    Set_mode(CONFIG_MODE);
    uart_write_bytes(LORA_UART_NUM, (const char *) send, 6);
}

void Lora_send_data(uint8_t *data) {
    // char len[12];
    done = 1;
    Busy_check();

    // sprintf(len, "%d", strlen((char *) data));
    // uart_write_bytes(LORA_UART_NUM, (const char *) len, strlen(len));
    // vTaskDelay(10 / portTICK_PERIOD_MS);
    uart_write_bytes(LORA_UART_NUM, (const char *) data, strlen((char *) data));
}

void Lora_update_req(void *pvP) {
    char *mess;
    while (1) {
        int64_t start_time = esp_timer_get_time();
        while (done) {
            if (esp_timer_get_time() - start_time >= 2000000) done = 0; // 2 seconds
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if (Bell_status) {
            mess = Sensor_json_handler(1, "1", 0, 0, 1);
            Bell_status = 0;
        } else {
            mess = Sensor_json_handler(1, "1", 0, 0, 0);
        }
        Lora_send_data((uint8_t *)mess);
        start_time = esp_timer_get_time();
        // if (Bell_status) {
        //     Bell_status = 0;
        //     while (done) {
        //         if (esp_timer_get_time() - start_time >= 2000000) done = 0; // 2 seconds
        //     }
        //     mess = Bell_json_handler(1, 1);
        //     if (mess != NULL) {
        //         Lora_send_data((uint8_t *)mess);
        //         free(mess);
        //     }
        // }
        // start_time = esp_timer_get_time();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// ffmpeg -rtsp_transport tcp -i "rtsp://admin:9813CA6F@192.168.0.108:554/1/2" ^
// -t 10 -c:v libx264 -preset ultrafast -f mp4 -y "C:\xampp\htdocs\camera\stream_10s.mp4"