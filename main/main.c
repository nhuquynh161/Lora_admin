#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "gpio.h"
#include "uart_lora.h"
#include "Smart_config.h"
#include "MQTT.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    gpio_init();
    UART_event_config();
    // Lora_set_address(0x000000, 23);
    // Lora_get_info();
    Wifi_init();
    xTaskCreate(LED_smartconfig_blink, "LED blink function", 1024, NULL, 3, NULL);
    xTaskCreate(Button_func, "Button function", 1024 * 4, NULL, 3, NULL);
    xTaskCreate(Lora_update_req, "Lora update req", 1024 * 3, NULL, 5, NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    MQTT_start();

    Set_mode(NORMAL_MODE);

}