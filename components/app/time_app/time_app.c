#include "time_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/cdefs.h>
#include <stdio.h>
#include <time.h>
#include <esp_log.h>
#include <sys/time.h>
#include "lvgl/lvgl.h"
#include "http.h"


lv_obj_t *ui_Screen1;
lv_obj_t *img_time;
lv_obj_t *img_s;
lv_obj_t *img_f;
lv_obj_t *img_m;

typedef struct Anim_data_t {
    lv_obj_t *target;
    int val;
} Anim_data_t;

void anim_ImgAngle_cb(struct _lv_anim_t *anim, int32_t val) {
    Anim_data_t *userData = anim->user_data;
    lv_obj_t *target = userData->target;
    lv_img_set_angle(target, (int16_t) val);
}

///////////////////// ANIMATIONS ////////////////////
static void circle_Animation(lv_obj_t *TargetObject, int start, int end) {

    Anim_data_t *userData = lv_mem_alloc(sizeof(Anim_data_t));
    userData->target = TargetObject;
    userData->val = -1;
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_time(&anim, 800);
    lv_anim_set_user_data(&anim, userData);
    lv_anim_set_custom_exec_cb(&anim, anim_ImgAngle_cb);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_repeat_count(&anim, 1);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_start(&anim);
}


void app_ui() {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);    /// Flags

    img_time = lv_img_create(ui_Screen1);
    lv_img_set_src(img_time, "S:/time.bin");
    lv_obj_set_align(img_time, LV_ALIGN_CENTER);

    img_s = lv_img_create(ui_Screen1);
    lv_img_set_src(img_s, "S:/s.png");
    lv_obj_set_x(img_s, 0);
    lv_obj_set_y(img_s, -28);
    lv_obj_set_align(img_s, LV_ALIGN_CENTER);
    lv_img_set_pivot(img_s, 2, 58);
    lv_img_set_angle(img_s, 0);

    img_f = lv_img_create(ui_Screen1);
    lv_img_set_src(img_f, "S:/f.png");
    lv_obj_set_x(img_f, 0);
    lv_obj_set_y(img_f, -35);
    lv_obj_set_align(img_f, LV_ALIGN_CENTER);
    lv_img_set_pivot(img_f, 1, 70);
    lv_img_set_angle(img_f, 0);

    img_m = lv_img_create(ui_Screen1);
    lv_img_set_src(img_m, "S:/m.png");
    lv_obj_set_x(img_m, 0);
    lv_obj_set_y(img_m, -40);
    lv_obj_set_align(img_m, LV_ALIGN_CENTER);
    lv_img_set_pivot(img_m, 1, 94);
    lv_img_set_angle(img_m, 0);

    lv_disp_load_scr(ui_Screen1);
}


void setTime(time_t timestamp) {

    struct tm *timeinfo = NULL;
    timeinfo = localtime(&timestamp);

    // 输出解析得到的时间信息（可选）
//    printf("Year: %d\n", timeinfo->tm_year + 1900);
//    printf("Month: %02d\n", timeinfo->tm_mon + 1);
//    printf("Day: %02d\n", timeinfo->tm_mday);
//    printf("Hour: %02d\n", timeinfo->tm_hour);
//    printf("Minute: %02d\n", timeinfo->tm_min);
//    printf("Second: %02d\n", timeinfo->tm_sec);

    // 设置系统时间
    time_t timeSinceEpoch = mktime(timeinfo);
    if (timeSinceEpoch == -1) {
        ESP_LOGI(TAG, "error,设置时间错误");
    } else {
        struct timeval now = {.tv_sec = timeSinceEpoch};
        settimeofday(&now, NULL);
        ESP_LOGI(TAG, "success:时间设置成功");
    }
//    if (timeinfo != NULL) {
//        free(timeinfo);
//        timeinfo = NULL;
//    }
}

int haveTimestamp() {
    char *result = http_get("http://vv.video.qq.com/checktime?otype=json", "");

    int tiSt = -1;
    char timeStamp[11];

    printf("aa\n");
//    if (result)
    if (strcmp(result, "no data") != 0) {
        strncpy(timeStamp, result + 26, 10); // 使用strncpy()截取字符串
        tiSt = atoi(timeStamp);
    }
    if (result != NULL) {
        free(result); // 释放内存
        result = NULL;
    }
    printf("bbb\n");

    return tiSt;
}

void updateTimeUI() {

    static time_t now = -1;
    static int lastHour = 0, lastMin = 0, lastSec = 0;
    struct tm timeinfo;

    if (now == -1) {
        now = haveTimestamp();
        if (now > 0) {
            setTime(now);
        }
        return;
    }

    time(&now);
    localtime_r(&now, &timeinfo);


    if (lastHour != timeinfo.tm_hour) {
        int startAngle = lastHour * 300;
        int endAngle = timeinfo.tm_hour % 12 * 300;
        if (endAngle == 0) {
            endAngle = 12 * 300;
        }
        circle_Animation(img_s, startAngle, endAngle);
        lastHour = timeinfo.tm_hour;
    }
    if (lastMin != timeinfo.tm_min) {
        int startAngle = lastMin * 60;
        int endAngle = timeinfo.tm_min * 60;
        if (endAngle == 0) {
            endAngle = 60 * 60;
        }
        circle_Animation(img_f, startAngle, endAngle);
        lastMin = timeinfo.tm_min;
    }
    if (lastSec != timeinfo.tm_sec) {
        int startAngle = lastSec * 60;
        int endAngle = timeinfo.tm_sec * 60;
        if (endAngle == 0) {
            endAngle = 60 * 60;
        }
        circle_Animation(img_m, startAngle, endAngle);
        lastSec = timeinfo.tm_sec;
    }

}

_Noreturn IRAM_ATTR void time_app(void *arg) {
    setTime(890582400);
    setenv("TZ", "CST-8", 1);    // 将时区设置为中国标准时间
    tzset();

    app_ui();

    lv_timer_create(updateTimeUI, 1000, NULL);

    TickType_t delayTime = pdMS_TO_TICKS(10); // 延迟100ms秒
    while (true) {
        vTaskDelay(delayTime);
        //http://vv.video.qq.com/checktime?otype=json
        // https://api.thinkpage.cn/v3/weather/now.json?key=g3egns3yk2ahzb0p&location=jinan&language=en
        lv_task_handler();//屏幕刷新和事件


    }
}
