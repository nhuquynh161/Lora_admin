#ifndef UART_LORA_H
#define UART_LORA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gpio.h"
#include "driver/uart.h"

#define LORA_UART_NUM UART_NUM_2
#define BAUD_RATE 9600
#define TXD GPIO_NUM_17
#define RXD GPIO_NUM_16
#define PATTERN_CHR_NUM    (3)

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

#define NORMAL_MODE 0
#define CONFIG_MODE 1
#define WAKEUP_MODE 2
#define SAVE_MODE 3

extern char tem[10], hum[10], air[10];
extern bool new_lora;

void UART_event_config(void);
void Set_mode(uint8_t mode);
void Lora_get_info(void);
void Lora_set_address(uint16_t address, uint8_t channel);
void Lora_send_data(uint8_t *data);
void Lora_update_req(void *pvP);

#endif // UART_LORA_H