
#ifndef HOLOC_UI_TIME_H
#define HOLOC_UI_TIME_H

#include "stdio.h"
#include "lvgl/lvgl.h"
#include <sys/time.h>

void time_ui_init();

void rotate_Animation(lv_obj_t *TargetObject, int32_t, int32_t);

void setTime(time_t timestamp);

void update_time_ui();

#endif