//
// Created by acer-zgy on 2023/7/7.
//


#ifndef HOLOC_LCD_SPI_H
#define HOLOC_LCD_SPI_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏
#define LCD_W 240
#define LCD_H 240

#define PIN_MOSI 23
#define PIN_SCK 18
#define PIN_DC 2
#define PIN_RES 4
#define PIN_BLK 5

#define LCD_DC_Clr() gpio_set_level(PIN_DC,0);
#define LCD_DC_Set() gpio_set_level(PIN_DC,1);


#define LCD_RES_Clr() gpio_set_level(PIN_RES,0);
#define LCD_RES_Set()  gpio_set_level(PIN_RES,1);

#define LCD_BLK_Clr()  gpio_set_level(PIN_BLK,1);
#define LCD_BLK_Set()  gpio_set_level(PIN_BLK,0);

void Lcd_Init(void);

void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color);

void clear(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);

#endif //HOLOC_LCD_SPI_H
