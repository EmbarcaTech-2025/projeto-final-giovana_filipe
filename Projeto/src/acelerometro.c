// acelerometro.c - Leitura do MPU6050
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/mpu6050_i2c.h"

void init_acelerometro() {
    mpu6050_setup_i2c();
    mpu6050_reset();
}

void ler_acelerometro(char* buffer, size_t len) {
    int16_t accel[3], gyro[3], temp;
    mpu6050_read_raw(accel, gyro, &temp);
    snprintf(buffer, len, "X:%d Y:%d Z:%d", accel[0], accel[1], accel[2]);
}
