// acelerometro.c - Leitura do MPU6050
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/mpu6050_i2c.h"

static i2c_inst_t *mpu6050_i2c = i2c0;

void init_acelerometro() {
    // Inicializa o I2C e os pinos
    i2c_init(mpu6050_i2c, 400000);
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);
    
    // Inicializa o MPU6050
    mpu6050_init(mpu6050_i2c);
}

void ler_acelerometro(char* buffer, size_t len) {
    float x, y, z;
    mpu6050_read_accel(&x, &y, &z);
    snprintf(buffer, len, "X:%.2f Y:%.2f Z:%.2f", x, y, z);
}
