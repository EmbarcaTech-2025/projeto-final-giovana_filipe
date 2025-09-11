#include "aht10.h"

// comandos
#define AHT10_CMD_INIT    0xE1
#define AHT10_CMD_MEASURE 0xAC
#define AHT10_CMD_SOFT_RESET 0xBA

void aht10_init(i2c_inst_t *i2c) {
    uint8_t cmd_init[] = {AHT10_CMD_INIT, 0x08, 0x00};
    // tentativa simples; ignorar retorno para não travar init
    i2c_write_blocking(i2c, AHT10_I2C_ADDR, cmd_init, 3, false);
    sleep_ms(40);
}

bool aht10_read_data(i2c_inst_t *i2c, float *humidity, float *temperature) {
    if (!i2c || !humidity || !temperature) return false;

    uint8_t cmd_measure[] = {AHT10_CMD_MEASURE, 0x33, 0x00};
    uint8_t status;
    uint8_t data[6];

    if (i2c_write_blocking(i2c, AHT10_I2C_ADDR, cmd_measure, 3, false) < 0) {
        return false;
    }
    sleep_ms(80);

    // Ler status primeiro (1 byte) e esperar até que o bit de busy (bit7) seja 0
    for (int tries = 0; tries < 10; ++tries) {
        if (i2c_read_blocking(i2c, AHT10_I2C_ADDR, &status, 1, false) >= 0) {
            if ((status & 0x80) == 0) break;
        }
        sleep_ms(10);
    }

    // Ler 6 bytes de dados
    if (i2c_read_blocking(i2c, AHT10_I2C_ADDR, data, 6, false) < 0) {
        return false;
    }

    // Umidade: 20 bits (data[1..3] bits[19:4])
    uint32_t humidity_raw = ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
    humidity_raw >>= 4; // align
    *humidity = (float)humidity_raw * 100.0f / (1 << 20);

    // Temperatura: 20 bits (data[3] low nibble + data[4], data[5])
    uint32_t temp_raw = (((uint32_t)(data[3] & 0x0F)) << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
    *temperature = (float)temp_raw * 200.0f / (1 << 20) - 50.0f;

    return true;
}
