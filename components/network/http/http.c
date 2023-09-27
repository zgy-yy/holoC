
#include "http.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "wifi.h"

static char *TAG = "HTTP_CLIENT";

static char *res_data;


// http客户端的事件处理回调函数
static esp_err_t http_client_event_handler(esp_http_client_event_t *evt) {
    static int len = 0;
    static char *res_buffer;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "connected to web-server");
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {

                int64_t resLen = esp_http_client_get_content_length(evt->client);
                if (res_buffer == NULL) {
                    res_buffer = (char *) malloc(resLen);
                    len = 0;
                    if (res_buffer == NULL) {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(res_buffer + len, evt->data, evt->data_len);
                len += evt->data_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "finished a request and response!");

            break;
        case HTTP_EVENT_DISCONNECTED:
            if (res_data != NULL) {
                free(res_data);
                res_data = NULL;
            }

            if (res_buffer != NULL) {
                res_data = (char *) malloc(len);
                memcpy(res_data, res_buffer, len);
                free(res_buffer);
                res_buffer = NULL;
                len = 0;
            }
            ESP_LOGI(TAG, "disconnected to web-server");
            break;
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "error");
            break;
        default:
            break;
    }

    return ESP_OK;
}

char *http_get(char *url, char *params) {
    char *res;
    res = (char *) malloc(8);
    strcpy(res, "no data");
// 获取默认网络接口
    char *ip = getIp();
    if (strcmp(ip, "0.0.0.0") == 0) {
        return res;
    }

    esp_err_t ret;
    const esp_http_client_config_t cfg = {
            .method=HTTP_METHOD_GET,
            .url = url,
            .event_handler = http_client_event_handler
    };
    //使用http服务器配置参数对http客户端初始化
    esp_http_client_handle_t httpClient = esp_http_client_init(&cfg);

    ret = esp_http_client_perform(httpClient);//启动并阻塞等待一次响应完成
    if (ret == ESP_OK) {
        int64_t content_length = esp_http_client_get_content_length(httpClient);
        ESP_LOGI(TAG, "Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(httpClient),//状态码
                 content_length);//数据长度
    } else {
        ESP_LOGE(TAG, "HTTP  request failed: %s", esp_err_to_name(ret));
    }
    esp_http_client_cleanup(httpClient);

    if (res_data != NULL) {
        res = (char *) malloc(strlen(res_data));
        strcpy(res, res_data);
    }
    return res;
}

void http_post(char *url, char *postData) {
    const esp_http_client_config_t cfg = {
            .method=HTTP_METHOD_POST,
            .url = url,
            .event_handler = http_client_event_handler
    };
    //使用http服务器配置参数对http客户端初始化
    esp_http_client_handle_t httpClient = esp_http_client_init(&cfg);
    esp_http_client_set_post_field(httpClient, postData, strlen(postData));

    esp_err_t ret = esp_http_client_perform(httpClient);//启动并阻塞等待一次响应完成

    if (ret == ESP_OK) {
        int64_t content_length = esp_http_client_get_content_length(httpClient);
        ESP_LOGI(TAG, "Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(httpClient),//状态码
                 content_length);//数据长度
    } else {
        ESP_LOGE(TAG, "HTTP  request failed: %s", esp_err_to_name(ret));
    }

    esp_http_client_cleanup(httpClient);
}