#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "inc/bmp280.h"
#include "inc/mpu6050_i2c.h"
#include "inc/luminosidade.h"

#define SDA_OLED 14
#define SCL_OLED 15
#define BUTTON_A 5
#define JOY_X 26
#define JOY_Y 27
#define DEADZONE 1000
#define BH1750_ADDR 0x23
#define BH1750_CONT_HRES_MODE 0x10

i2c_inst_t *oled_i2c = i2c1;
i2c_inst_t *sensor_i2c = i2c0;

typedef enum {
    TELA_BEM_VINDO,
    MENU_PRINCIPAL,
    MENU_TEMPERATURA,
    MENU_PRESSAO,
    MENU_LUMINOSIDADE,
    MENU_ACELEROMETRO,
    MENU_FINALIZAR
} EstadoMenu;

EstadoMenu estado = TELA_BEM_VINDO;

// === Desenho do coração ===
void ssd1306_draw_heart(uint8_t *ssd, int x, int y) {
    const uint8_t heart[8][8] = {
        {0,1,1,0,0,1,1,0},
        {1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1},
        {0,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,0,0},
    };
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (heart[i][j])
                ssd1306_set_pixel(ssd, x + j, y + i, true);
}


// Função para desenhar um coração na matriz 5x5
void draw_heart(uint8_t r, uint8_t g, uint8_t b) {
    int heart[5][5] = {
        {1, 0, 1, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 1, 0, 0}
    };

    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            if (heart[y][x]) {
                npSetLED(x, y, r, g, b);
            } else {
                npSetLED(x, y, 0, 0, 0);
            }
        }
    }
    npwrite();
}

// === Funções de display ===
void display_linhas(const char* l1, const char* l2, const char* l3, const char* l4) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 0, 0,  l1);
    ssd1306_draw_string(buffer, 0, 16, l2);
    ssd1306_draw_string(buffer, 0, 32, l3);
    ssd1306_draw_string(buffer, 0, 48, l4);
    render_on_display(buffer, &area);
}

void ssd1306_draw_filled_rect(uint8_t *ssd, int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++)
        for (int j = y; j < y + h; j++)
            ssd1306_set_pixel(ssd, i, j, true);
}

void display_bem_vindo_com_heart() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 20, 24, "Bem-vindos ao");
    ssd1306_draw_string(buffer, 20, 36, "BioCooler");
    ssd1306_draw_heart(buffer, 2, 28);
    render_on_display(buffer, &area);
}

void display_mensagem_central(const char* msg) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 10, 28, msg);
    render_on_display(buffer, &area);
}

void display_menu_principal(int menu_idx) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);

    ssd1306_draw_string(buffer, 0, 0, "Menu BioCooler:");

    const char* opcoes[] = {
        "Temperatura",
        "PRESSAO",
        "Luminosidade",
        "Acelerometro",
        "Finalizar"
    };

    for (int i = 0; i < 5; i++) {
        if (menu_idx == i)
            ssd1306_draw_filled_rect(buffer, 0, 16 + i * 10, 6, 8);
        ssd1306_draw_string(buffer, 10, 16 + i * 10, opcoes[i]);
    }

    render_on_display(buffer, &area);
}

// === Inicialização ===
void init_display() {
    i2c_init(oled_i2c, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);
    ssd1306_init();
}

bool btn_a_pressionado() {
    static uint32_t ultimo_ms = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());
    if (!gpio_get(BUTTON_A)) {
        if (agora - ultimo_ms > 50) {
            ultimo_ms = agora;
            return true;
        }
    }
    return false;
}

int8_t joystick_direcao() {
    static int8_t ultimo_dir = 0;
    adc_select_input(1);
    uint16_t y = adc_read();
    int8_t dir = 0;
    if (y < DEADZONE) dir = 1;
    else if (y > 4000) dir = -1;
    if (dir != ultimo_dir && dir != 0) {
        ultimo_dir = dir;
        return dir;
    } else if (dir == 0) {
        ultimo_dir = 0;
    }
    return 0;
}

void init_luminosidade() {
    uint8_t cmd = BH1750_CONT_HRES_MODE;
    i2c_write_blocking(sensor_i2c, BH1750_ADDR, &cmd, 1, false);
}

void init_hw() {
    stdio_init_all();
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);
    init_display();

    i2c_init(sensor_i2c, 100000);
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    mpu6050_setup_i2c();
    mpu6050_reset();
    bmp280_init();
    init_luminosidade();
}

// === Main ===
int main() {
    init_hw();
    sleep_ms(1000);
    int menu_idx = 0;

    while (1) {
        switch (estado) {
            case TELA_BEM_VINDO:
                display_bem_vindo_com_heart();
                sleep_ms(2000);

                display_mensagem_central("Digite a senha");
                while (!btn_a_pressionado()) sleep_ms(100);

                display_mensagem_central("A iniciar transporte");
                sleep_ms(2000);

                estado = MENU_PRINCIPAL;
                break;

            case MENU_PRINCIPAL: {
                display_menu_principal(menu_idx);
                int8_t dir = joystick_direcao();
                if (dir == 1) menu_idx = (menu_idx + 4) % 5;
                else if (dir == -1) menu_idx = (menu_idx + 1) % 5;

                if (btn_a_pressionado()) {
                    switch (menu_idx) {
                        case 0: estado = MENU_TEMPERATURA; break;
                        case 1: estado = MENU_PRESSAO; break;
                        case 2: estado = MENU_LUMINOSIDADE; break;
                        case 3: estado = MENU_ACELEROMETRO; break;
                        case 4: estado = MENU_FINALIZAR; break;
                    }
                }
                sleep_ms(150);
                break;
            }

            case MENU_TEMPERATURA: {
                while (1) {
                    sensors_t dados = bmp280_get_all(0x76);
                    char t[22];
                    snprintf(t, sizeof(t), "Temp: %.1f C", dados.temperature);
                    display_linhas("Temperatura", t, "", "< A para voltar");
                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    sleep_ms(200);
                }
                break;
            }

            case MENU_PRESSAO: {
                while (1) {
                    sensors_t dados = bmp280_get_all(0x76);
                    char u[22];
                    snprintf(u, sizeof(u), "Press: %.1f hPa", dados.pressure / 100.0);
                    display_linhas("PRESSAO", u, "", "< A para voltar");
                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    sleep_ms(200);
                }
                break;
            }

            case MENU_LUMINOSIDADE: {
                while (1) {
                    uint16_t lux = ler_lux();
                    char luxstr[22];
                    snprintf(luxstr, sizeof(luxstr), "Luminosidade: %d lux", lux);
                    display_linhas("Sensor BH1750", luxstr, "", "< A para voltar");
                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    sleep_ms(200);
                }
                break;
            }

            case MENU_ACELEROMETRO: {
                while (1) {
                    int16_t acc[3], gyr[3], temp;
                    mpu6050_read_raw(acc, gyr, &temp);
                    char acel[22];
                    snprintf(acel, sizeof(acel), "X:%d Y:%d Z:%d", acc[0], acc[1], acc[2]);
                    display_linhas("Acelerometro", acel, "", "< A para voltar");
                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    sleep_ms(200);
                }
                break;
            }

            case MENU_FINALIZAR: {
                display_mensagem_central("Finalizando...");
                sleep_ms(2000);
                estado = MENU_PRINCIPAL;
                break;
            }
        }
    }
}

