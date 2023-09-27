
#ifndef HOLOC_URI_HANDLER_H
#define HOLOC_URI_HANDLER_H

#include "esp_err.h"
#include <esp_http_server.h>

typedef struct ParamKV {
    char key[30];
    char val[30];
} ParamKV;

typedef struct URI_INFO {
    char path[60];
    char type[6];
    ParamKV params[6];
    int paramLen;
} URI_INFO;

void get_uri_info(char *uri, URI_INFO *);

esp_err_t sendFile(httpd_req_t *req, char *filepath, char *);

esp_err_t file_handler(httpd_req_t *req);


esp_err_t ws_handler(httpd_req_t *req);

esp_err_t scan_aps(httpd_req_t *req);

esp_err_t wifi_set_ap(httpd_req_t *req);

#endif