#include <stdio.h>
#include "lcd_spi.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "sd_dev.h"

IRAM_ATTR void lvgl_task(void *arg) {
//    lv_demo_music();
    //lv_demo_benchmark();

    /* 打开文件 */
    lv_fs_file_t file;
    lv_fs_res_t res = lv_fs_open(&file, "S:/hello.txt", LV_FS_MODE_RD);

    if (res != LV_FS_RES_OK) {
        lv_fs_close(&file);
        return;
    }

    /* 读取文件内容 */
    char buffer[60 + 1];
    lv_fs_read(&file, buffer, 60, NULL);

    /* 关闭文件 */
    lv_fs_close(&file);
//
//
    LV_FONT_DECLARE(myFont);

    static lv_style_t label_style;                                    // 创建一个风格
    lv_style_init(&label_style);                                    // 初始化风格
    lv_style_set_text_font(&label_style, &myFont);// 设置风格的字体
    lv_obj_t *textlabel = lv_label_create(lv_scr_act());            // 在主屏幕创建一个标签
    lv_obj_set_width(textlabel, 240);
    lv_obj_set_height(textlabel, 240);   // 设置标签宽度
    lv_label_set_recolor(textlabel, true);                                // 使能字符命令重新对字符上色
    lv_label_set_text(textlabel, "这是SD卡的文字");    // 设置显示文本
    lv_obj_add_style(textlabel, &label_style, LV_STATE_DEFAULT);        // 应用效果风格

//    lv_obj_t *ui_Image2 = lv_img_create(lv_scr_act());
//    lv_img_set_src(ui_Image2, "S:/a.png");

    while (true) {
        vTaskDelay(((10) / portTICK_PERIOD_MS));
        lv_task_handler();
        printf("context：\n%s\n", buffer);
    }
}


void app_main() {
    SD_Init();
    Lcd_Init();
    lv_init();
    lv_port_disp_init(); // lvgl显示接口初始化

//    static lv_obj_t *src;
//    src = lv_scr_act();
//    lv_obj_t *label = lv_label_create(src);
//    lv_label_set_text(label, "hello");
//    lv_obj_center(label);
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, "S:/Blustery_33.bin");


    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 4096, NULL, 2, NULL, APP_CPU_NUM);
}