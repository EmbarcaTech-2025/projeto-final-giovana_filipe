#ifndef AHT10_H
#define AHT10_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>

#define AHT10_I2C_ADDR 0x38

void aht10_init(i2c_inst_t *i2c);
bool aht10_read_data(i2c_inst_t *i2c, float *humidity, float *temperature);

#endif // AHT10_H
