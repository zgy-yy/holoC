#ifndef HOLOC_KALMEN_H
#define HOLOC_KALMEN_H

#include "mpu_iic.h"

typedef struct D_rollpitch {
    float roll;
    float pitch
} D_roll_pitch_t;

D_roll_pitch_t getMpu_data();

void update_event();

#endif