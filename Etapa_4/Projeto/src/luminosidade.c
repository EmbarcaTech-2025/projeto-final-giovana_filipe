// luminosidade.c - Leitura do BH1750
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define BH1750_ADDR 0x23
#define BH1750_CONT_HRES_MODE 0x10
// Use the same I2C instance as main.c
static i2c_inst_t* sensor_i2c = i2c0;

void init_luminosidade() {
    uint8_t cmd = BH1750_CONT_HRES_MODE;
    i2c_write_blocking(sensor_i2c, BH1750_ADDR, &cmd, 1, false);
}

uint16_t ler_lux() {
    uint8_t data[2];
    i2c_read_blocking(sensor_i2c, BH1750_ADDR, data, 2, false);
    uint16_t raw = (data[0] << 8) | data[1];
    return raw / 1.2;
}

float get_luminosidade() {
    return (float)ler_lux();
}
