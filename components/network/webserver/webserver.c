
#include <sys/param.h>
#include "webserver.h"
#include "esp_log.h"
#include "handlers/uri_handler.h"

static char *TAG = "HTTP_SERVER";


/* HTTP服务器请求处理函数 */
esp_err_t root_get_handler(httpd_req_t *req) {

    char *filename = "/S/webserver/index.html"; // 设置要发送的文件路径

    sendFile(req, filename, "html");

    return ESP_OK;
}


/* 配置HTTP服务器 */
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;


    //    web socket
    httpd_uri_t ws = {
            .uri="/ws",
            .method=HTTP_GET,
            .handler =ws_handler,
            .is_websocket=true,
            .user_ctx  = NULL
    };



    // 设置URL路径及请求方法对应的处理函数
    httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
    };
//    获取静态资源
    httpd_uri_t assets = {
            .uri       = "/assets/*",
            .method    = HTTP_GET,
            .handler   = file_handler,
            .user_ctx  = NULL
    };

// wifi 任务
    httpd_uri_t scanAps = {
            .uri="/wifi/scanAps",
            .method=HTTP_GET,
            .handler =scan_aps,
            .user_ctx  = NULL,
    };
    httpd_uri_t setAp = {
            .uri="/wifi/setAp",
            .method=HTTP_POST,
            .handler =wifi_set_ap,
            .user_ctx  = NULL
    };

    // 创建HTTP服务器实例
    if (httpd_start(&server, &config) == ESP_OK) {

//        注册 websocket 处理函数
        httpd_register_uri_handler(server, &ws);
        // 将处理函数注册到HTTP服务器实例中
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &assets);
        httpd_register_uri_handler(server, &scanAps);

        httpd_register_uri_handler(server, &setAp);
    } else {
        return NULL;
    }




    return server;
}

/* 停止并销毁HTTP服务器 */
void stop_webserver(httpd_handle_t server) {
    if (server) {
        httpd_stop(server);
    }
}