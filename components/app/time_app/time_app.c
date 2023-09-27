#include "time_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/cdefs.h>
#include <stdio.h>
#include <esp_log.h>
#include <sys/time.h>
#include "lvgl/lvgl.h"
#include "http.h"
#include "drivers/Mpu6050/mpu_iic.h"
#include "drivers/Mpu6050/kalmen_filter.h"


#define TAG  "time_app"



_Noreturn IRAM_ATTR void time_app(void *arg) {
    setTime(890582400);
    setenv("TZ", "CST-8", 1);    // 将时区设置为中国标准时间
    tzset();

    time_ui_init();

    lv_timer_create(update_time_ui, 1000, NULL);


    TickType_t delayTime = pdMS_TO_TICKS(100); // 延迟100ms秒


    while (true) {
        vTaskDelay(delayTime);
        //http://vv.video.qq.com/checktime?otype=json
        // https://api.thinkpage.cn/v3/weather/now.json?key=g3egns3yk2ahzb0p&location=jinan&language=en
        lv_task_handler();//屏幕刷新和事件

        update_event();
    }
}
