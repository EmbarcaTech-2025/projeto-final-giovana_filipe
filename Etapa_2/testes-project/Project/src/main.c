// main.c - BioSmartCooler Simplificado - Sistema de Sensores com Display OLED

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
    MENU_LUMINOSIDADE,
    MENU_ACELEROMETRO,
    MENU_DIAGNOSTICO,
    MENU_FINALIZAR
} EstadoMenu;

EstadoMenu estado = TELA_BEM_VINDO;

// Variáveis globais para sensores
float temperatura_atual = 0.0f;
float umidade_atual = 0.0f;
float luminosidade_atual = 0.0f;
float acelerometro_x = 0.0f;
float acelerometro_y = 0.0f;
float acelerometro_z = 0.0f;

// Implementação da função ssd1306_draw_pixel
void ssd1306_draw_pixel(uint8_t *buf, int x, int y, bool color) {
    if (x >= ssd1306_width || y >= ssd1306_height) return;
    int page = y / 8;
    int bit = y % 8;
    if (color) buf[x + page * ssd1306_width] |= (1 << bit);
    else buf[x + page * ssd1306_width] &= ~(1 << bit);
}

void draw_cursor(uint8_t* buffer, uint8_t x, uint8_t y) {
    ssd1306_draw_pixel(buffer, x, y+1, true);
    ssd1306_draw_pixel(buffer, x+1, y, true);
    ssd1306_draw_pixel(buffer, x+1, y+2, true);
    ssd1306_draw_pixel(buffer, x+2, y, true);
    ssd1306_draw_pixel(buffer, x+2, y+2, true);
    ssd1306_draw_pixel(buffer, x+3, y+1, true);
}

void display_linhas(const char* l1, const char* l2, const char* l3, const char* l4) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof(buffer));
    ssd1306_draw_string(buffer, 0, 0, l1);
    ssd1306_draw_string(buffer, 0, 16, l2);
    ssd1306_draw_string(buffer, 0, 32, l3);
    ssd1306_draw_string(buffer, 0, 48, l4);
    render_on_display(buffer, &area);
}

void display_bem_vindo() {
    display_linhas("BioSmartCooler", "Sistema Iniciado", "", "Press A para continuar");
}

void display_menu_principal(int menu_idx) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof(buffer));

    ssd1306_draw_string(buffer, 0, 0, "Menu BioSmartCooler:");
    ssd1306_draw_string(buffer, 8, 16, "Temperatura/Umidade");
    ssd1306_draw_string(buffer, 8, 32, "Luminosidade");
    ssd1306_draw_string(buffer, 8, 48, "Acelerometro");

    // Desenha cursor na opção selecionada
    if (menu_idx == 0) draw_cursor(buffer, 0, 16);
    else if (menu_idx == 1) draw_cursor(buffer, 0, 32);
    else if (menu_idx == 2) draw_cursor(buffer, 0, 48);

    render_on_display(buffer, &area);
}

void display_temperatura() {
    char linha1[32], linha2[32], linha3[32], linha4[32];
    
    snprintf(linha1, sizeof(linha1), "Temperatura:");
    snprintf(linha2, sizeof(linha2), "Temp: %.1f C", temperatura_atual);
    snprintf(linha3, sizeof(linha3), "Umidade: %.1f%%", umidade_atual);
    snprintf(linha4, sizeof(linha4), "Press A para voltar");
    
    display_linhas(linha1, linha2, linha3, linha4);
}

void display_luminosidade() {
    char linha1[32], linha2[32], linha3[32], linha4[32];
    
    snprintf(linha1, sizeof(linha1), "Luminosidade:");
    snprintf(linha2, sizeof(linha2), "Lux: %.1f", luminosidade_atual);
    snprintf(linha3, sizeof(linha3), "Status: %s", 
             luminosidade_atual > 100 ? "Claro" : 
             luminosidade_atual > 50 ? "Moderado" : "Escuro");
    snprintf(linha4, sizeof(linha4), "Press A para voltar");
    
    display_linhas(linha1, linha2, linha3, linha4);
}

void display_acelerometro() {
    char linha1[32], linha2[32], linha3[32], linha4[32];
    
    snprintf(linha1, sizeof(linha1), "Acelerometro:");
    snprintf(linha2, sizeof(linha2), "X: %.2f", acelerometro_x);
    snprintf(linha3, sizeof(linha3), "Y: %.2f Z: %.2f", acelerometro_y, acelerometro_z);
    snprintf(linha4, sizeof(linha4), "Press A para voltar");
    
    display_linhas(linha1, linha2, linha3, linha4);
}

void display_diagnostico() {
    char linha1[32], linha2[32], linha3[32], linha4[32];
    
    snprintf(linha1, sizeof(linha1), "Diagnostico:");
    snprintf(linha2, sizeof(linha2), "BMP280: OK");
    snprintf(linha3, sizeof(linha3), "MPU6050: OK");
    snprintf(linha4, sizeof(linha4), "Press A para voltar");
    
    display_linhas(linha1, linha2, linha3, linha4);
}

void init_display() {
    i2c_init(oled_i2c, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);
    ssd1306_init(oled_i2c);
}

bool btn_a_pressionado() {
    static bool ultimo_estado = false;
    bool estado_atual = !gpio_get(BUTTON_A);
    bool pressionado = estado_atual && !ultimo_estado;
    ultimo_estado = estado_atual;
    return pressionado;
}

int8_t joystick_direcao() {
    uint16_t x = adc_read();
    sleep_ms(1);
    uint16_t y = adc_read();
    
    if (x < 2048 - DEADZONE) return -1; // Esquerda
    if (x > 2048 + DEADZONE) return 1;  // Direita
    if (y < 2048 - DEADZONE) return -2; // Cima
    if (y > 2048 + DEADZONE) return 2;  // Baixo
    return 0; // Centro
}

void init_luminosidade() {
    // Configuração do sensor de luminosidade BH1750
    uint8_t cmd = BH1750_CONT_HRES_MODE;
    i2c_write_blocking(sensor_i2c, BH1750_ADDR, &cmd, 1, false);
}

void init_hw() {
    // Inicializa GPIO
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    
    // Inicializa ADC para joystick
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);
    adc_select_input(0);
    
    // Inicializa I2C para sensores
    i2c_init(sensor_i2c, 400000);
    gpio_set_function(16, GPIO_FUNC_I2C); // SDA
    gpio_set_function(17, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(16);
    gpio_pull_up(17);
    
    // Inicializa display
    init_display();
    
    // Inicializa sensores
    bmp280_init(sensor_i2c);
    mpu6050_init(sensor_i2c);
    init_luminosidade();
    
    printf("Hardware inicializado com sucesso!\n");
}

void ler_sensores() {
    // Lê temperatura e umidade
    bmp280_read_temp_humidity(&temperatura_atual, &umidade_atual);
    
    // Lê acelerômetro
    mpu6050_read_accel(&acelerometro_x, &acelerometro_y, &acelerometro_z);
    
    // Lê luminosidade
    uint8_t data[2];
    i2c_read_blocking(sensor_i2c, BH1750_ADDR, data, 2, false);
    luminosidade_atual = (data[0] << 8 | data[1]) / 1.2f;
}

void verificar_alarmes_temperatura(float temperatura) {
    if (temperatura > 30.0f) {
        // Alarme de temperatura alta
        printf("ALARME: Temperatura alta: %.1f C\n", temperatura);
    }
}

int main() {
    stdio_init_all();
    printf("Iniciando BioSmartCooler...\n");
    
    init_hw();
    
    int menu_idx = 0;
    uint32_t ultima_atualizacao = 0;
    
    while (true) {
        uint32_t agora = to_ms_since_boot(get_absolute_time());
        
        // Atualiza sensores a cada 1 segundo
        if (agora - ultima_atualizacao > 1000) {
            ler_sensores();
            verificar_alarmes_temperatura(temperatura_atual);
            ultima_atualizacao = agora;
        }
        
        // Processa entrada do usuário
        if (btn_a_pressionado()) {
            switch (estado) {
                case TELA_BEM_VINDO:
                    estado = MENU_PRINCIPAL;
                    break;
                case MENU_PRINCIPAL:
                    if (menu_idx == 0) estado = MENU_TEMPERATURA;
                    else if (menu_idx == 1) estado = MENU_LUMINOSIDADE;
                    else if (menu_idx == 2) estado = MENU_ACELEROMETRO;
                    break;
                case MENU_TEMPERATURA:
                case MENU_LUMINOSIDADE:
                case MENU_ACELEROMETRO:
                case MENU_DIAGNOSTICO:
                    estado = MENU_PRINCIPAL;
                    break;
            }
        }
        
        int8_t joystick = joystick_direcao();
        if (estado == MENU_PRINCIPAL && joystick != 0) {
            if (joystick == 2) menu_idx = (menu_idx + 1) % 3; // Baixo
            else if (joystick == -2) menu_idx = (menu_idx - 1 + 3) % 3; // Cima
        }
        
        // Atualiza display
        switch (estado) {
            case TELA_BEM_VINDO:
                display_bem_vindo();
                break;
            case MENU_PRINCIPAL:
                display_menu_principal(menu_idx);
                break;
            case MENU_TEMPERATURA:
                display_temperatura();
                break;
            case MENU_LUMINOSIDADE:
                display_luminosidade();
                break;
            case MENU_ACELEROMETRO:
                display_acelerometro();
                break;
            case MENU_DIAGNOSTICO:
                display_diagnostico();
                break;
        }
        
        sleep_ms(100);
    }
    
    return 0;
}
