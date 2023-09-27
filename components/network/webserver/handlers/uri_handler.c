#include <esp_log.h>
#include <cJSON.h>
#include "uri_handler.h"
#include "wifi.h"

#define Tag "server handler"

void get_uri_info(char *uri, URI_INFO *uriInfo) {

    memset(uriInfo, 0, sizeof(URI_INFO));

    char *path = strtok(uri, "?");
    strcpy(uriInfo->path, path);

    char *sub = strtok(NULL, "");
    strtok(path, ".");

    char *type = strtok(NULL, "");
    if (type != NULL) {
        strcpy(uriInfo->type, type);
    }
    char *paramStr = strtok(sub, "&");
    int index = 0;
    while (paramStr != NULL) {
        sub = strtok(NULL, "");

        strcpy(uriInfo->params[index].key, strtok(paramStr, "="));
        strcpy(uriInfo->params[index].val, strtok(NULL, ""));
        index++;
        paramStr = strtok(sub, "&");
    }
    uriInfo->paramLen = index;
}

char *hasHttpType(char *type) {
    if (strcmp(type, "html") == 0) {
        return "text/html;charset=UTF-8";
    }
    if (strcmp(type, "js") == 0) {
        return "application/javascript";
    }
    if (strcmp(type, "css") == 0) {
        return "text/css";
    }
    if (strcmp(type, "svg") == 0) {
        return "image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8";
    }
    if (strcmp(type, "json") == 0) {
        return "application/json;charset=UTF-8";
    }
    return "*/*";
}

esp_err_t sendFile(httpd_req_t *req, char *filepath, char *type) {

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        ESP_LOGE(Tag, "Failed to open file for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, hasHttpType(type));
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
//        printf("%s", line);
        if (httpd_resp_send_chunk(req, line, len) != ESP_OK) {
            fclose(fp);
            ESP_LOGE(Tag, "File sending failed!");
            return ESP_FAIL;
        }
    }
    httpd_resp_send_chunk(req, NULL, 0);
    fclose(fp);

    return ESP_OK;
}

esp_err_t file_handler(httpd_req_t *req) {
    URI_INFO uriInfo = {};
//    printf("请求的地址:%s\n", req->uri);

    get_uri_info((char *) req->uri, &uriInfo);
    char *filepath = (char *) malloc(sizeof(char) * 100);
    sprintf(filepath, "%s%s", "/S/webserver", uriInfo.path);
//    printf("文件路径:%s\n", filepath);

    sendFile(req, filepath, uriInfo.type);

    free(filepath);

    return ESP_OK;
}


//web socket handler
esp_err_t ws_handler(httpd_req_t *req) {

    if (req->method == HTTP_GET) {
        ESP_LOGI("ws", "handshake done ,new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_kpt;
    uint8_t *buf = NULL;
    memset(&ws_kpt, 0, sizeof(httpd_ws_frame_t));
    ws_kpt.type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_kpt, 0);
    if (ret != ESP_OK) {
        ESP_LOGI("ws", "http_ws failed to get frame");
        return ret;
    }
    if (ws_kpt.len) {
        buf = calloc(1, ws_kpt.len + 1);
        if (buf == NULL) {
            ESP_LOGE("ws", "failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_kpt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_kpt, ws_kpt.len);
        if (ret != ESP_OK) {
            ESP_LOGE("ws", "http_ws recv failed with %d", ret);
            free(buf);
            return ret;
        }

        ESP_LOGI("ws", "got packet with message: %s", ws_kpt.payload);
    }
    ESP_LOGI("ws", "packet type %d", ws_kpt.type);

    ret = httpd_ws_send_frame(req, &ws_kpt);
    if (ret != ESP_OK) {
        ESP_LOGE("ws", "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);

    return ret;
}

esp_err_t scan_aps(httpd_req_t *req) {

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//    设置返回类型
    httpd_resp_set_type(req, hasHttpType("json"));

    char *data_json = wifiScanAP();

    httpd_resp_send(req, data_json, strlen(data_json));

    free(data_json);

    return ESP_OK;
}

esp_err_t wifi_set_ap(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (req->method == HTTP_POST) {

// 获取POST请求数据的长度
        int content_len = req->content_len;

        // 创建一个缓冲区来存储POST请求数据
        char *buf = malloc(content_len + 1);
        memset(buf, 0, content_len + 1);

        // 从请求中读取POST请求数据
        int ret = httpd_req_recv(req, buf, content_len);

        if (ret <= 0) {
            // 处理读取错误
            free(buf);
            return ESP_FAIL;
        }

        // 处理POST请求数据
        printf("Received POST data: %s\n", buf);
        cJSON* root = cJSON_Parse(buf);
        char *name = cJSON_GetObjectItem(root, "name")->valuestring;
        char *password = cJSON_GetObjectItem(root, "password")->valuestring;

        wifi_connect_ap(name, password);

        return ESP_OK;
    }

    return ESP_FAIL;
}