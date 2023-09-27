#include <dhcpserver/dhcpserver.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <freertos/event_groups.h>
#include "wifi.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "string.h"
#include "lwip/lwip_napt.h"
#include "cJSON.h"

static const char *Tag = "WIFI";

// 事件组允许每个事件有多个位，这里只使用两个事件：
// 已经连接到AP并获得了IP
// 在最大重试次数后仍未连接
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_CONNECTING_BIT BIT2
// FreeRTOS事件组，连接成功时发出信号
static EventGroupHandle_t s_wifi_event_group = NULL;


void enable_dhcp(esp_netif_t *netif, esp_netif_ip_info_t *ip_info) {
    esp_netif_dhcp_status_t status;
    esp_netif_dhcps_get_status(netif, &status);
    printf("dhcp status  %d\n", status);

}


void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(Tag, "sta mode start");
            esp_wifi_connect();
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTING_BIT);
        }
        if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI(Tag, "wifi disconnected");
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }



//        ap 事件
        if (event_id == WIFI_EVENT_AP_START) {
            ESP_LOGI(Tag, "AP mode start");
        }
        if (event_id == WIFI_EVENT_AP_STACONNECTED) {
            ESP_LOGI(Tag, "one Device Connected To This Ap!");
        }
        if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
            ESP_LOGI(Tag, "one Device Loss Connected!");
        }
        if (event_id == WIFI_EVENT_AP_STOP) {
            ESP_LOGI(Tag, "AP mode stop");

        }
    }

    if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
            ESP_LOGI(Tag, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }


}


void wifiInit() {
//    初始化 nvs存储
    nvs_flash_init();
//wifi 初始化
    esp_netif_init();
    esp_event_loop_create_default();//创建一个默认的事件循环

    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifiInitConfig);

    esp_wifi_set_storage(WIFI_STORAGE_FLASH);

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

//    创建并初始化 sta对象
    wifi_config_t cfg_sta = {
            .sta={
                    .ssid = "CQUPT",
                    .password = ""
            },
    };
    wifi_config_t cfg_ap = {
            .ap={
                    .ssid=AP_SSID,
                    .ssid_len =strlen(AP_SSID),
                    .password = AP_PASS,
                    .channel=12,
                    .max_connection = 3,
                    .authmode = WIFI_AUTH_WPA2_PSK
            }
    };

//    esp_wifi_get_config(WIFI_IF_STA, &cfg_sta);

    // 如果配置为空，则配置并保存WiFi信息
//    if (strlen((const char *) cfg_sta.sta.ssid) == 0) {
//        strncpy((char *) cfg_sta.sta.ssid, "CQUPT", 6);
//        strncpy((char *) cfg_sta.sta.password, "", 1);
//    }
    printf("ss : %s", cfg_sta.sta.ssid);

    esp_wifi_set_config(WIFI_IF_STA, &cfg_sta);
    esp_wifi_set_config(WIFI_IF_AP, &cfg_ap);

    esp_wifi_set_mode(WIFI_MODE_APSTA); //设置WiFi 工作模式

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, sta_netif, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, ap_netif, NULL);


    esp_wifi_start();
    // 创建一个事件组，用于管理Wi-Fi连接事件。
    s_wifi_event_group = xEventGroupCreate();

}


char *wifiScanAP() {


    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT | WIFI_CONNECTING_BIT,
                                           pdFALSE, pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() 返回调用前的 bits，因此我们可以测试实际发生了什么事件。 */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI("AP", "connected to ap ");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI("AP", "Failed to connect to ");
    } else if (bits & WIFI_CONNECTING_BIT) {
        ESP_LOGE("AP", "connecting ap");
        return "{array:[]}";
    }

    wifi_country_t scan_config = {
            .cc="CN",
            .schan=1,
            .nchan =12,
            .policy=WIFI_COUNTRY_POLICY_AUTO
    };
    esp_wifi_set_country(&scan_config);
    wifi_scan_config_t scanConfig = {
            .show_hidden=true          /**< enable to scan AP whose SSID is hidden */
    };
    esp_wifi_scan_start(&scanConfig, true); // true 扫描是阻塞
    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);

    wifi_ap_record_t *ap_list = (wifi_ap_record_t *) malloc(sizeof(wifi_ap_record_t) * ap_num);
    memset(ap_list, 0, sizeof(wifi_ap_record_t));

    uint16_t aps_count = ap_num;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&aps_count, ap_list));

    ESP_LOGI("WIFI", "AP Count: %d", aps_count);

//    printf("%30s %s %s %s\n", "SSID", "频道", "强度", "MAC地址");

    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    cJSON *item;
    for (int i = 0; i < aps_count; i++) {
        item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "name", (char *) ap_list[i].ssid);
        cJSON_AddNumberToObject(item, "strength", ap_list[i].rssi);
        cJSON_AddItemToArray(array, item);
//
        printf("%30s  %3d  %3d  %02X-%02X-%02X-%02X-%02X-%02X\n", ap_list[i].ssid, ap_list[i].primary,
               ap_list[i].rssi, ap_list[i].bssid[0], ap_list[i].bssid[1], ap_list[i].bssid[2],
               ap_list[i].bssid[3], ap_list[i].bssid[4], ap_list[i].bssid[5]);
    }

    cJSON_AddItemToObject(root, "array", array);

    char *json_str = cJSON_Print(root);

//    free(json_str);
    free(ap_list);
    cJSON_Delete(root);

    return json_str;
}


void wifi_connect_ap(char *ssid, char *password) {
//    先断开连接
    esp_wifi_disconnect();
    // 配置STA模式
    wifi_config_t cfg_sta = {};
    strncpy((char *) cfg_sta.sta.ssid, ssid, strlen(ssid));
    strncpy((char *) cfg_sta.sta.password, password, strlen(password));

    esp_wifi_set_config(WIFI_IF_STA, &cfg_sta);

    esp_wifi_connect();
}

char *getIp() {
    // 获取默认网络接口
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    // 获取IP地址信息
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);

    return ip4addr_ntoa(&ip_info.ip);
}