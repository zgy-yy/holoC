#include <sys/cdefs.h>
#include "lcd_spi.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "sd_dev.h"
#include "wifi.h"
#include "time_app.h"


void app_main() {
    SD_Init();
    Lcd_Init();
    lv_init();
    lv_port_disp_init(); // lvgl显示接口初始化


    wifiInit();

//    wifiScanAP();

    xTaskCreatePinnedToCore(time_app, "time_app", 4096, NULL, 2, NULL, APP_CPU_NUM);
}