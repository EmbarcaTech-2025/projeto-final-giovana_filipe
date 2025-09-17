#include "inc/display.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/keypad.h"

static i2c_inst_t *oled_i2c = i2c1;

void init_display() {
    i2c_init(oled_i2c, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);
    sleep_ms(100);
    ssd1306_init();
    clear_display();
    printf("Display OLED inicializado\n");
}

void clear_display() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof(buffer));
    render_on_display(buffer, &area);
}

void display_bem_vindo() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 20, 20, "Bem-vindo ao");
    ssd1306_draw_string(buffer, 30, 35, "BioCooler");
    render_on_display(buffer, &area);
}

void display_mensagem_central(const char* msg) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    char* lines[3] = {NULL, NULL, NULL};
    char temp_msg[64];
    strncpy(temp_msg, msg, sizeof(temp_msg));
    char* token = strtok(temp_msg, "\n");
    int line_count = 0;
    while (token != NULL && line_count < 3) {
        lines[line_count++] = token;
        token = strtok(NULL, "\n");
    }
    int start_y;
    if (line_count == 1) start_y = 28;
    else if (line_count == 2) start_y = 22;
    else start_y = 16;
    for (int i = 0; i < line_count; i++) {
        if (lines[i] != NULL) {
            int text_width = strlen(lines[i]) * 6;
            int x_pos = (ssd1306_width - text_width) / 2;
            ssd1306_draw_string(buffer, x_pos, start_y + i * 12, lines[i]);
        }
    }
    render_on_display(buffer, &area);
}

void display_linhas(const char* l1, const char* l2, const char* l3, const char* l4) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    if (l1) ssd1306_draw_string(buffer, 0, 0,  l1);
    if (l2) ssd1306_draw_string(buffer, 0, 16, l2);
    if (l3) ssd1306_draw_string(buffer, 0, 32, l3);
    if (l4) ssd1306_draw_string(buffer, 0, 48, l4);
    render_on_display(buffer, &area);
}

void display_menu_principal(int menu_idx) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);

    if (menu_idx < scroll_offset) {
        scroll_offset = menu_idx;
    } else if (menu_idx >= scroll_offset + max_visible_items) {
        scroll_offset = menu_idx - max_visible_items + 1;
    }

    ssd1306_draw_string(buffer, 0, 0, "Menu BioCooler:");
    char* opcoes[] = {
        "Temperatura",
        "Pressao",
        "Luminosidade",
        "Acelerometro",
        "Umidade",            // << NOVO
        "Controle Tampa",
        "Config. Tempo",
        "Travar/Destravar",
        "Finalizar"
    };
    int total_opcoes = 9;
    for (int i = 0; i < max_visible_items; i++) {
        int item_idx = scroll_offset + i;
        if (item_idx >= total_opcoes) break;
        int y_pos = 16 + i * 12;
        if (menu_idx == item_idx) {
            ssd1306_draw_filled_circle(buffer, 4, y_pos + 4, 3);
        }
        ssd1306_draw_string(buffer, 10, y_pos, opcoes[item_idx]);
    }

    if (scroll_offset > 0) {
        ssd1306_draw_string(buffer, 120, 0, "^");
    }
    if (scroll_offset + max_visible_items < total_opcoes) {
        ssd1306_draw_string(buffer, 120, 56, "v");
    }
    render_on_display(buffer, &area);
}

void display_config_tempo(uint32_t horas, uint32_t minutos, bool editando_horas) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 0, 0, "Config. Tempo Entrega");
    char tempo_str[32];
    snprintf(tempo_str, sizeof(tempo_str), "%02lu:%02lu", horas, minutos);
    int text_width = strlen(tempo_str) * 6;
    int x_pos = (ssd1306_width - text_width) / 2;
    ssd1306_draw_string(buffer, x_pos, 20, tempo_str);
    if (editando_horas) {
        ssd1306_draw_string(buffer, x_pos, 30, "^^");
    } else {
        ssd1306_draw_string(buffer, x_pos + 25, 30, "^^");
    }
    ssd1306_draw_string(buffer, 0, 40, "Joystick: Ajustar");
    ssd1306_draw_string(buffer, 0, 50, "A: Campo  B: Confirmar");
    render_on_display(buffer, &area);
}

void display_config_tempo_alerta(uint32_t minutos_alerta) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 0, 0, "Config. Tempo Alerta");
    char alerta_str[32];
    snprintf(alerta_str, sizeof(alerta_str), "Alerta: %02lu min antes", minutos_alerta);
    ssd1306_draw_string(buffer, 10, 16, alerta_str);
    ssd1306_draw_string(buffer, 0, 32, "Joystick: ajustar valor");
    ssd1306_draw_string(buffer, 0, 48, "B: confirmar");
    render_on_display(buffer, &area);
}

void display_controle_tampa(int estado_caixa) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    ssd1306_draw_string(buffer, 0, 0, "Controle da Tampa");
    if (estado_caixa) {
        ssd1306_draw_string(buffer, 0, 20, "Tampa: ABERTA");
    } else {
        ssd1306_draw_string(buffer, 0, 20, "Tampa: FECHADA");
    }
    ssd1306_draw_string(buffer, 0, 40, "A: Travar/Destravar");
    render_on_display(buffer, &area);
}

// telas de sensores simples
void display_temperatura(float temperatura) {
    char buf[32];
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    snprintf(buf, sizeof(buf), "Temperatura: %.1f C", temperatura);
    ssd1306_draw_string(buffer, 0, 20, buf);
    render_on_display(buffer, &area);
}

void display_pressao(float pressao) {
    char buf[32];
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    snprintf(buf, sizeof(buf), "Pressao: %.2f hPa", pressao);
    ssd1306_draw_string(buffer, 0, 20, buf);
    render_on_display(buffer, &area);
}

void display_luminosidade(uint16_t lux) {
    char buf[32];
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    snprintf(buf, sizeof(buf), "Luminosidade: %u lux", lux);
    ssd1306_draw_string(buffer, 0, 20, buf);
    render_on_display(buffer, &area);
}

void display_acelerometro(int16_t acc_x, int16_t acc_y, int16_t acc_z) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    char line1[32], line2[32], line3[32];
    snprintf(line1, sizeof(line1), "Acc X:%d Y:%d", acc_x, acc_y);
    snprintf(line2, sizeof(line2), "Acc Z:%d", acc_z);
    ssd1306_draw_string(buffer, 0, 8, line1);
    ssd1306_draw_string(buffer, 0, 24, line2);
    render_on_display(buffer, &area);
}

void display_humidade(float umidade, float temperatura) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);

    char linha1[32], linha2[32];
    snprintf(linha1, sizeof(linha1), "Umidade: %.1f %%", umidade);
    snprintf(linha2, sizeof(linha2), "Temp: %.1f C", temperatura);

    ssd1306_draw_string(buffer, 0, 20, linha1);
    ssd1306_draw_string(buffer, 0, 40, linha2);

    render_on_display(buffer, &area);
}
// Funções gráficas auxiliares
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

void ssd1306_draw_filled_circle(uint8_t *ssd, int x_center, int y_center, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                ssd1306_set_pixel(ssd, x_center + x, y_center + y, true);
            }
        }
    }
}