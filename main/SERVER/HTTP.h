#ifndef HTTP_H
#define HTTP_H

#include "esp_http_client.h"

extern bool send_;

esp_err_t send_http_request(char *tem, char *hum, char *air, int bell);
esp_err_t send_database_http_request(char *tem, char *hum, char *air, int bell);

#endif // HTTP_H