

#include "wifi.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "string.h"

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    TickType_t delayTime = pdMS_TO_TICKS(1000); // 延迟100ms秒

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(Tag, "connect ap");
        esp_wifi_connect();
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(Tag, "wifi disconnect ,connect ap");
        esp_wifi_connect();
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(Tag, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }

}

void wifiInit() {
//    初始化 nvs存储
    nvs_flash_init();
//wifi 初始化
    esp_netif_init();
    esp_event_loop_create_default();//创建一个默认的事件循环
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifiInitConfig);

    wifi_sta_config_t cfg_sta = {
            .ssid = "Iphone zy",
            .password = "00000000",
    };
    esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t *) &cfg_sta);
    esp_wifi_set_mode(WIFI_MODE_STA); //设置WiFi 工作模式

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL);
    esp_wifi_start();
}

void wifiScanAP() {
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
    ESP_LOGI("wifi", "ap count:%d", ap_num);

    uint16_t max_aps = 20;
    wifi_ap_record_t ap_records[max_aps];
    memset(ap_records, 0, sizeof(ap_records));

    uint16_t aps_count = max_aps;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&aps_count, ap_records));

    ESP_LOGI("WIFI", "AP Count: %d", aps_count);

    printf("%30s %s %s %s\n", "SSID", "频道", "强度", "MAC地址");

    for (int i = 0; i < aps_count; i++) {
        printf("%30s  %3d  %3d  %02X-%02X-%02X-%02X-%02X-%02X\n", ap_records[i].ssid, ap_records[i].primary,
               ap_records[i].rssi, ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2],
               ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
    }
}

