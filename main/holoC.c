#include <sys/cdefs.h>
#include "lcd_spi.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "sd_dev.h"
#include "wifi.h"
#include "mpu_iic.h"
#include "webserver.h"
#include "time_app.h"
#include "weather_app.h"

void app_main() {

    init_mpu6050();

    SD_Init();
    Lcd_Init();
    lv_init();
    lv_port_disp_init(); // lvgl显示接口初始化
    lv_port_indev_init();

//    屏幕显示初始化
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    wifiInit();
    start_webserver();

    xTaskCreatePinnedToCore(time_app, "time_app", 4086, NULL, 2, NULL, APP_CPU_NUM);

}
