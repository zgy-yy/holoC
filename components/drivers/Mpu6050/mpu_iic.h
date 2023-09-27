#ifndef HOLOC_MPU6050_H

#define HOLOC_MPU6050_H

#include "stdio.h"

#define Tag "MPU6050"

#define MPU6050_I2C_PORT_NUM 0

#define MPU_I2C_SDA 32
#define MPU_I2C_SCL 33

#define MPU6050_I2C_FREQ 400000


#define MPU6050_ADDR 0x68  //器件地址： b110100(AD0)

#define WRITE_BIT I2C_MASTER_WRITE  //I2C master write
#define READ_BIT I2C_MASTER_READ    //I2C master read

#define ACK_CHECK_EN 0x1            //I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0           //I2C master will not check ack from slave
#define ACK_VAL 0x0                 //I2C ack value
#define NACK_VAL 0x1                //I2C nack value


typedef struct accel_data {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
} accel_data_t;

typedef struct temp_data {
    int16_t temp;
} temp_data_t;

typedef struct gyro_data {
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} gyro_data_t;

typedef struct measurement_data {
    accel_data_t accel_data;
    temp_data_t temp_data;
    gyro_data_t gyro_data;
} mpu_data_t;


void init_mpu6050();

mpu_data_t mpu6050_get_value();


#endif