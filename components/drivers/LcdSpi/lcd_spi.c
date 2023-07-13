//
// Created by acer-zgy on 2023/7/7.
//

#include "lcd_spi.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <stdio.h>
#include <driver/gpio.h>

#define TAG "SPI_LCD"

void delay_ms(int i) {
    vTaskDelay(i / portTICK_PERIOD_MS);
}


static spi_device_handle_t spi;

extern void spi_disp_flush_ready(void);


IRAM_ATTR  void spi_ready(spi_transaction_t *trans) {
    uint32_t spi_cnt = (uint32_t) trans->user;

    if (spi_cnt == 4) {
        spi_disp_flush_ready();
    }
}


static const spi_bus_config_t busConfig = {
        .miso_io_num=-1,
        .mosi_io_num=PIN_MOSI,
        .sclk_io_num =PIN_SCK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4094
};

static const spi_device_interface_config_t deviceConfig = {
        .clock_speed_hz=SPI_MASTER_FREQ_40M,
        .mode=3,
        .spics_io_num = -1,
        .queue_size =80,
        .cs_ena_pretrans = 1,
        .post_cb = spi_ready,//注册一个SPI调用完成的回调
};

void vspi_init() {
    spi_bus_initialize(SPI3_HOST, &busConfig, SPI_DMA_CH1);
    spi_bus_add_device(SPI3_HOST, &deviceConfig, &spi);
}

void spi_tans_data(const uint8_t cmd) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8;                     //Command is 8 bits
    t.tx_buffer = &cmd;               //The data is the cmd itself
    t.user = (void *) 0;                //D/C needs to be set to 0
    ret = spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret == ESP_OK);            //Should have had no issues.
}

void LCD_WR_REG(uint8_t dat) {
    LCD_DC_Clr();//写命令
    spi_tans_data(dat);
}

void LCD_WR_DATA8(uint8_t dat) {
    LCD_DC_Set();//写数据
    spi_tans_data(dat);
}

void LCD_WR_DATA(uint16_t dat) {
    LCD_DC_Set();//写数据
    spi_tans_data(dat >> 8);
    spi_tans_data(dat);
}

void LCD_WR_DATA16(uint16_t dat) {
    LCD_DC_Set();//写数据
    spi_tans_data(dat >> 8);
    spi_tans_data(dat);
}


IRAM_ATTR void VSPI_data_queue(uint16_t *dat, uint32_t len, uint32_t user_fg) {
    static spi_transaction_t t[80];
    static uint32_t i = 0;

    memset(&t[i], 0, sizeof(spi_transaction_t)); // Zero out the transaction
    t[i].length = len;                           // Command is 8 bits
    t[i].tx_buffer = dat;                        // The data is the cmd itself
    t[i].user = (void *) user_fg;                 // D/C needs to be set to 0
    esp_err_t ret = spi_device_queue_trans(spi, &t[i], portMAX_DELAY);
    assert(ret == ESP_OK); // Should have had no issues.
    i++;
    if (i == 80) {
        i = 0;
    }
}

void gpioInit() {
    gpio_reset_pin(PIN_DC);
    gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_RES);
    gpio_set_direction(PIN_RES, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_BLK);
    gpio_set_direction(PIN_BLK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_DC, 0);
    gpio_set_level(PIN_RES, 0);
    gpio_set_level(PIN_BLK, 0);
}


void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    if (USE_HORIZONTAL == 0) {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2c);//储存器写
    } else if (USE_HORIZONTAL == 1) {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1 + 80);
        LCD_WR_DATA(y2 + 80);
        LCD_WR_REG(0x2c);//储存器写
    } else if (USE_HORIZONTAL == 2) {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2c);//储存器写
    } else {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1 + 80);
        LCD_WR_DATA(x2 + 80);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2c);//储存器写
    }
}

void Lcd_Init() {
    gpioInit();
    vspi_init();

    LCD_RES_Clr();
    delay_ms(100);
    LCD_RES_Set();
    delay_ms(100);
    LCD_BLK_Set();
    delay_ms(100);
    //************* Start Initial Sequence **********//
    LCD_WR_REG(0x11); //Sleep out
    delay_ms(120);              //Delay 120ms
    //************* Start Initial Sequence **********//
    LCD_WR_REG(0x36);
    if (USE_HORIZONTAL == 0)LCD_WR_DATA8(0x00);
    else if (USE_HORIZONTAL == 1)LCD_WR_DATA8(0xC0);
    else if (USE_HORIZONTAL == 2)LCD_WR_DATA8(0x70);
    else LCD_WR_DATA8(0xA0);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);

    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x19);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x2C);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x12);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);

    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x0F);

    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x54);
    LCD_WR_DATA8(0x4C);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x23);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x20);
    LCD_WR_DATA8(0x23);
    LCD_WR_REG(0x21);

    LCD_WR_REG(0x29);
}


void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color) {
    uint32_t siofs = 240 * 2 * 10; //同时刷多少行240 *2（8bit）*10行
    uint32_t siof = 240 * 1 * 10;  //因为数组是16位的，但字节指向的地址
    uint32_t size = (x2 + 1 - x1) * (y2 + 1 - y1) * 2;
    LCD_Address_Set(x1, y1, x2, y2);
    LCD_DC_Set();                       //写数据
    uint32_t send_cnt = size / siofs;  //需要整行刷新多少行
    uint32_t send_cnt2 = size % siofs; //整行刷不完还剩多少字节
    if (size == 0) {
        ESP_LOGE(TAG, "size为0,跳出了");
        return;
    }
    for (int i = 0; i < send_cnt; i++) {
        if ((i + 1) == send_cnt && send_cnt2 == 0) {
            VSPI_data_queue(&color[i * siof], siof * 16, 4);
        } else {
            VSPI_data_queue(&color[i * siof], siof * 16, 3);
        }
    }
    if (send_cnt2 != 0) {
        VSPI_data_queue(&color[send_cnt * siof], send_cnt2 * 8, 4);
    }
}

void clear(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color) {
    uint16_t i, j;
    LCD_Address_Set(xsta, ysta, xend - 1, yend - 1);
    for (i = ysta; i < yend; i++) {
        for (j = xsta; j < xend; j++) {
            LCD_WR_DATA(color);
        }
    }
}