#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"
#include <cJSON.h>

// #define BROKER_URL "mqtt://192.168.1.44"
#define BROKER_URL "mqtt://MSI.local"
// #define BROKER_URL "mqtt://lapbotlinux.local"

#define TOPIC_PUB_SENSOR "/bao_chay/sensor_st"
#define TOPIC_SUB_CTRL "/bao_chay/ctrl_a"

#define _BELL "b"

extern int Bell_status, Bell;

extern esp_mqtt_client_handle_t _client;

// char* Bell_json_handler(int dev, int bell);
char* Sensor_json_handler(int dev, char *tem, char *hum, char *air, int bell);
void MQTT_start(void);

#endif // MQTT_H