#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/bmp280.h"

// Pinos conectados ao BMP280 (usando I2C0)
#define BMP280_SDA_PIN 0
#define BMP280_SCL_PIN 1
#define BMP280_ADDR 0x76

static i2c_inst_t *bmp280_i2c = i2c0;

void temperatura_init() {
    // Inicializa o I2C e os pinos
    i2c_init(bmp280_i2c, 100000);
    gpio_set_function(BMP280_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BMP280_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BMP280_SDA_PIN);
    gpio_pull_up(BMP280_SCL_PIN);

    // Inicializa o sensor BMP280
    bmp280_init();
}

// Função para retornar os dados do sensor
sensors_t ler_bmp280() {
    return bmp280_get_all(BMP280_ADDR);
}
