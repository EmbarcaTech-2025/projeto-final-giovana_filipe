#include "./mpu6050_i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define MPU6050_ADDR 0x68

// Variável global para armazenar a instância I2C
static i2c_inst_t* mpu6050_i2c = NULL;

// Função simplificada para inicializar MPU6050
void mpu6050_init(i2c_inst_t* i2c) {
    mpu6050_i2c = i2c;
    
    // Wake up MPU6050
    uint8_t buf[] = {0x6B, 0x00};
    i2c_write_blocking(mpu6050_i2c, MPU6050_ADDR, buf, 2, false);
    sleep_ms(100);
    
    // Configure accelerometer range (±2g)
    buf[0] = 0x1C;
    buf[1] = 0x00;
    i2c_write_blocking(mpu6050_i2c, MPU6050_ADDR, buf, 2, false);
    sleep_ms(10);
}

// Função simplificada para ler dados do acelerômetro
void mpu6050_read_accel(float* x, float* y, float* z) {
    if (mpu6050_i2c == NULL) {
        *x = *y = *z = 0.0f;
        return;
    }
    
    uint8_t buffer[6];
    uint8_t reg = 0x3B; // MPU6050_REG_ACCEL_XOUT_H
    
    i2c_write_blocking(mpu6050_i2c, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(mpu6050_i2c, MPU6050_ADDR, buffer, 6, false);
    
    int16_t accel_x = (buffer[0] << 8) | buffer[1];
    int16_t accel_y = (buffer[2] << 8) | buffer[3];
    int16_t accel_z = (buffer[4] << 8) | buffer[5];
    
    // Convert to float (assuming ±2g range)
    *x = accel_x / 16384.0f;
    *y = accel_y / 16384.0f;
    *z = accel_z / 16384.0f;
}

void mpu6050_setup_i2c() {
    i2c_init(I2C_PORT, 400*1000); // common options: 100*1000 (100 kHz) or 400*1000 (400 kHz)
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void mpu6050_reset() {
    uint8_t buf[] = {0x6B, 0x80};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
    sleep_ms(100);
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
    sleep_ms(10);
}

void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    uint8_t buffer[6];
    uint8_t reg = 0x3B; //MPU6050_REG_ACCEL_XOUT_H
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buffer, 6, false);
    for (int i=0; i<3; i++)
        accel[i] = (buffer[2*i]<<8) | buffer[2*i+1];

    reg = 0x43; //MPU6050_REG_GYRO_XOUT_H
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buffer, 6, false);
    for (int i=0; i<3; i++)
        gyro[i] = (buffer[2*i]<<8) | buffer[2*i+1];

    reg = 0x41; //MPU6050_REG_TEMP_OUT_H
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buffer, 2, false);
    *temp = (buffer[0]<<8) | buffer[1];
}