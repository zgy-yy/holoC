
#include <time.h>
#include <math.h>
#include "kalmen_filter.h"
#include "lv_api_map.h"
#include "lv_port_indev.h"
/*计算偏移量*/
//float i;                                    //计算偏移量时的循环次数
float ax_offset = 0, ay_offset = 0;         //x,y轴的加速度偏移量
float gx_offset = 0, gy_offset = 0;         //x,y轴的角速度偏移量

/*参数*/
float rad2deg = 57.29578;                   //弧度到角度的换算系数
float roll_v = 0, pitch_v = 0;              //换算到x,y轴上的角速度

/*定义微分时间*/
time_t now = 0, lasttime = 0;
float dt = 0;        //定义微分时间

/*三个状态，先验状态，观测状态，最优估计状态*/
float gyro_roll = 0, gyro_pitch = 0;        //陀螺仪积分计算出的角度，先验状态
float acc_roll = 0, acc_pitch = 0;          //加速度计观测出的角度，观测状态
float k_roll = 0, k_pitch = 0;              //卡尔曼滤波后估计出最优角度，最优估计状态

/*误差协方差矩阵P*/
float e_P[2][2] = {{1, 0},
                   {0, 1}};             //误差协方差矩阵，这里的e_P既是先验估计的P，也是最后更新的P

/*卡尔曼增益K*/
float k_k[2][2] = {{0, 0},
                   {0, 0}};             //这里的卡尔曼增益矩阵K是一个2X2的方阵


float sq(float x) {
    return x * x;
}

D_roll_pitch_t getMpu_data() {
    mpu_data_t m = mpu6050_get_value();

//    gyro_data_t g = {
//            .gyro_x=m.gyro_data.gyro_x,
//            .gyro_y = m.gyro_data.gyro_y,
//            .gyro_z = m.gyro_data.gyro_z
//    };

    accel_data_t a = {
            .accel_x = m.accel_data.accel_x,
            .accel_y = m.accel_data.accel_y,
            .accel_z = m.accel_data.accel_z
    };

    /*减去偏移量并根据加速度计算角度*/
    //roll角度
    k_roll = -1 * atan((a.accel_y - ay_offset) / (a.accel_z)) * rad2deg;

    //pitch角度
    k_pitch = atan((a.accel_x - ax_offset) / sqrt(sq(a.accel_y - ay_offset) + sq(a.accel_z))) * rad2deg;

    D_roll_pitch_t rp = {
            .roll = k_roll,
            .pitch = k_pitch
    };
    printf("%f/%f\n", k_roll, k_pitch);

    return rp;


//
//    time(&now);
//    dt = (now - lasttime) / 1000.0;           //微分时间(s)
//    printf("%lld,%f\n", now - lasttime, dt);
//    lasttime = now;
//
//
//    /*step1:计算先验状态*/
//    /*计算x,y轴上的角速度*/
//    roll_v = (g.gyro_x - gx_offset) + ((sin(k_pitch) * sin(k_roll)) / cos(k_pitch)) * (g.gyro_y - gy_offset) +
//             ((sin(k_pitch) * cos(k_roll)) / cos(k_pitch)) * g.gyro_z;//roll轴的角速度
//    pitch_v = cos(k_roll) * (g.gyro_y - gy_offset) - sin(k_roll) * g.gyro_z;//pitch轴的角速度
//    gyro_roll = k_roll + dt * roll_v;//先验roll角度
//    gyro_pitch = k_pitch + dt * pitch_v;//先验pitch角度
//
//    /*step2:计算先验误差协方差矩阵P*/
//    e_P[0][0] = e_P[0][0] + 0.0025;//这里的Q矩阵是一个对角阵且对角元均为0.0025
//    e_P[0][1] = e_P[0][1] + 0;
//    e_P[1][0] = e_P[1][0] + 0;
//    e_P[1][1] = e_P[1][1] + 0.0025;
//
//    /*step3:更新卡尔曼增益K*/
//    k_k[0][0] = e_P[0][0] / (e_P[0][0] + 0.3);
//    k_k[0][1] = 0;
//    k_k[1][0] = 0;
//    k_k[1][1] = e_P[1][1] / (e_P[1][1] + 0.3);
//
//    /*step4:计算最优估计状态*/
//    /*观测状态*/
//    //roll角度
//    acc_roll = atan((a.accel_y - ay_offset) / (a.accel_z)) * rad2deg;
//    //pitch角度
//    acc_pitch =
//            -1 * atan((a.accel_x - ax_offset) / sqrt(sq(a.accel_y - ay_offset) + sq(a.accel_z))) *
//            rad2deg;
//    /*最优估计状态*/
//    k_roll = gyro_roll + k_k[0][0] * (acc_roll - gyro_roll);
//    k_pitch = gyro_pitch + k_k[1][1] * (acc_pitch - gyro_pitch);
//
//    /*step5:更新协方差矩阵P*/
//    e_P[0][0] = (1 - k_k[0][0]) * e_P[0][0];
//    e_P[0][1] = 0;
//    e_P[1][0] = 0;
//    e_P[1][1] = (1 - k_k[1][1]) * e_P[1][1];
//
//    //打印角度
//    printf("%f,%f\n", k_roll, k_pitch);
}

extern lv_indev_state_t encoder_state;
extern int32_t encoder_diff;

void update_event() {
    D_roll_pitch_t rp = getMpu_data();

    if (rp.pitch < -16) {
        encoder_state = LV_INDEV_STATE_PR; //编码器按下
    } else {
        encoder_state = LV_INDEV_STATE_REL;
    }

    if (rp.roll < -18) {
        encoder_diff = -1;
    } else if (rp.roll > 18) {
        encoder_diff = 1;
    } else {
        encoder_diff = 0;
    }

}