#ifndef MPU6050_I2C_H // include guard
#define MPU6050_I2C_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Função simplificada para inicializar MPU6050
void mpu6050_init(i2c_inst_t* i2c);

// Função simplificada para ler dados do acelerômetro
void mpu6050_read_accel(float* x, float* y, float* z);

void mpu6050_setup_i2c(void);
void mpu6050_reset(void);
void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp);

#endif // MPU6050_I2C_H