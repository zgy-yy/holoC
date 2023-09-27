

#include <esp_log.h>
#include "ui_time.h"
#include "http.h"

#define TAG  "time_ui"

lv_obj_t *ui_Screen;
lv_obj_t *ui_plate;
lv_obj_t *ui_eli;
lv_obj_t *ui_hor;
lv_obj_t *ui_min;
lv_obj_t *ui_sec;

typedef struct _ui_anim_user_data_t {
    lv_obj_t *target;
    int32_t val;
} ui_anim_user_data_t;

void _ui_anim_callback_set_image_angle(lv_anim_t *a, int32_t v) {
    ui_anim_user_data_t *usr = (ui_anim_user_data_t *) a->user_data;
    lv_img_set_angle(usr->target, (int16_t) v);
}


void rotate_Animation(lv_obj_t *TargetObject, int32_t start, int32_t end) {
    ui_anim_user_data_t *ui_anim_ud = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    ui_anim_ud->target = TargetObject;
    ui_anim_ud->val = -1;
    lv_anim_t rotate_ani;
    lv_anim_init(&rotate_ani);
    lv_anim_set_time(&rotate_ani, 800);
    lv_anim_set_user_data(&rotate_ani, ui_anim_ud);

    lv_anim_set_custom_exec_cb(&rotate_ani, _ui_anim_callback_set_image_angle);

    lv_anim_set_values(&rotate_ani, start, end);
    lv_anim_set_path_cb(&rotate_ani, lv_anim_path_ease_in_out);
    lv_anim_set_delay(&rotate_ani, 0);

    lv_anim_set_repeat_count(&rotate_ani, 1);
    lv_anim_set_early_apply(&rotate_ani, false);
    lv_anim_start(&rotate_ani);

}

///////////////////// FUNCTIONS ////////////////////
void ui_event_Label7(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);

    if (event_code<20){
        printf("code %d\n", event_code);
        if (event_code == LV_EVENT_PRESSED) {
            printf("event pressed\n");
        }
    }
}

extern lv_indev_t *indev_encoder;

void time_ui_init() {

    lv_group_t *group;
    group = lv_group_create();
    lv_indev_set_group(indev_encoder, group);


    lv_disp_t *dispp = lv_disp_get_default();
    lv_disp_clean_dcache(dispp);

    ui_Screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen, LV_OBJ_FLAG_SCROLLABLE);    /// Flags

    ui_plate = lv_img_create(ui_Screen);
    lv_img_set_src(ui_plate, "S:/apps/time/plate.bin");
    lv_obj_set_width(ui_plate, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_plate, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align(ui_plate, LV_ALIGN_CENTER);


    ui_hor = lv_img_create(ui_Screen);
    lv_img_set_src(ui_hor, "S:/apps/time/hor.png");
    lv_obj_set_width(ui_hor, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_hor, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_y(ui_hor, -29);
    lv_obj_set_align(ui_hor, LV_ALIGN_CENTER);
    lv_img_set_pivot(ui_hor, 4, 54);
    lv_img_set_angle(ui_hor, 0);

    ui_min = lv_img_create(ui_Screen);
    lv_img_set_src(ui_min, "S:/apps/time/min.png");
    lv_obj_set_width(ui_min, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_min, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_y(ui_min, -41);
    lv_obj_set_align(ui_min, LV_ALIGN_CENTER);
    lv_img_set_pivot(ui_min, 4, 78);
    lv_img_set_angle(ui_min, 0);

    ui_sec = lv_img_create(ui_Screen);
    lv_img_set_src(ui_sec, "S:/apps/time/sec.png");
    lv_obj_set_width(ui_sec, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_sec, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_y(ui_sec, -44);
    lv_obj_set_align(ui_sec, LV_ALIGN_CENTER);
    lv_img_set_pivot(ui_sec, 2, 84);
    lv_img_set_angle(ui_sec, 900);


    ui_eli = lv_img_create(ui_Screen);
    lv_img_set_src(ui_eli, "S:/apps/time/eli.png");
    lv_obj_set_width(ui_eli, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_eli, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align(ui_eli, LV_ALIGN_CENTER);

    lv_group_add_obj(group, ui_eli);
    lv_obj_add_event_cb(ui_eli, ui_event_Label7, LV_EVENT_ALL, NULL);
    lv_disp_load_scr(ui_Screen);

}


int getTimestamp() {

    char *result = http_get("http://vv.video.qq.com/checktime?otype=json", "");

    int tiSt = -1;
    char timeStamp[11];

    if (strcmp(result, "no data") != 0) {
        strncpy(timeStamp, result + 26, 10); // 使用strncpy()截取字符串
        tiSt = atoi(timeStamp);
    }
    if (result != NULL) {
        free(result); // 释放内存
        result = NULL;
    }
    return tiSt;
}


void setTime(time_t timestamp) {

    struct tm *timeinfo = NULL;
    timeinfo = localtime(&timestamp);

    // 设置系统时间
    time_t timeSinceEpoch = mktime(timeinfo);
    if (timeSinceEpoch == -1) {
        ESP_LOGI(TAG, "error,设置时间错误");
    } else {
        struct timeval now = {.tv_sec = timeSinceEpoch};
        settimeofday(&now, NULL);
        ESP_LOGI(TAG, "success:时间设置成功");
    }

}

void update_time_ui() {
    static time_t now = -1;
    static int lastHour = 0, lastMin = 0, lastSec = 0;
    struct tm timeinfo;

    if (now == -1) {
        time_t begin;
        time_t end;
        time(&begin);
        now = getTimestamp();
        if (now > 0) {
            time(&end);
            setTime(now + end - begin);
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
        rotate_Animation(ui_hor, startAngle, endAngle);
        lastHour = timeinfo.tm_hour;
    }
    if (lastMin != timeinfo.tm_min) {
        int startAngle = lastMin * 60;
        int endAngle = timeinfo.tm_min * 60;
        if (endAngle == 0) {
            endAngle = 60 * 60;
        }
        rotate_Animation(ui_min, startAngle, endAngle);
        lastMin = timeinfo.tm_min;
    }
    if (lastSec != timeinfo.tm_sec) {
        int startAngle = lastSec * 60;
        int endAngle = timeinfo.tm_sec * 60;
        if (endAngle == 0) {
            endAngle = 60 * 60;
        }
        rotate_Animation(ui_sec, startAngle, endAngle);
        lastSec = timeinfo.tm_sec;
    }

}