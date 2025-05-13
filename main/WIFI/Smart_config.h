#ifndef SMART_CONFIG_H
#define SMART_CONFIG_H

#include "esp_wifi.h"
#include "esp_smartconfig.h"

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

void Wifi_init(void);
void LED_smartconfig_blink(void * parm);
void Button_func(void *pvParameter);

#endif // SMART_CONFIG_H