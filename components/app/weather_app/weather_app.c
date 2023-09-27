#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/cdefs.h>
#include <esp_log.h>
#include "lvgl/lvgl.h"
#include "weather_app.h"



_Noreturn void weather_app() {

    TickType_t delayTime = pdMS_TO_TICKS(100); // 延迟100ms秒
    while (true) {


        printf("weather");
        vTaskDelay(delayTime);
        //http://vv.video.qq.com/checktime?otype=json
        // https://api.thinkpage.cn/v3/weather/now.json?key=g3egns3yk2ahzb0p&location=jinan&language=en
        lv_task_handler();//屏幕刷新和事件
    }
}