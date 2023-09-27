
#include "ui_weather.h"


lv_obj_t *ui_Screen2;
lv_obj_t *ui_Label7;

void weather_ui_init() {
    ui_Screen2 = lv_obj_create(NULL);

    ui_Label7 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label7, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label7, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x(ui_Label7, -3);
    lv_obj_set_y(ui_Label7, -15);
    lv_obj_set_align(ui_Label7, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label7, "hello\n");
    lv_obj_set_style_bg_color(ui_Label7, lv_color_hex(0xA23333), LV_PART_MAIN | LV_STATE_DEFAULT);
}