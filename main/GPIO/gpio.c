#include "gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

void gpio_init(void) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = PININ_MAP;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = PINOUT_MAP;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_set_level(LED, 0);

    // while (1) {
    //     if (gpio_get_level(AUX) == 1) {
    //         ESP_LOGI("TAG", "I'm busy...");
    //     }
    //     vTaskDelay(10 / portTICK_PERIOD_MS);
    // }
}