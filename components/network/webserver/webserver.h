
#ifndef HOLOC_WEBSERVER_H
#define HOLOC_WEBSERVER_H

#include <esp_http_server.h>

httpd_handle_t start_webserver(void);

void stop_webserver(httpd_handle_t server);

#endif