#ifndef HTTP_H
#define HTTP_H

#include "esp_http_client.h"

#define PHP_savecam_URL "http://MSI.local/save_video_cam.php"
#define PHP_database_URL "http://MSI.local/database.php"

extern bool send_;

esp_err_t send_http_request();
esp_err_t send_database_http_request(char *tem, char *hum, char *air, int bell);

#endif // HTTP_H