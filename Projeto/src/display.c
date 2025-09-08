#include "inc/display.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/keypad.h"

// Variáveis globais do módulo
static i2c_inst_t *oled_i2c = i2c1;


void init_display() {
    // Inicializa o I2C
    i2c_init(oled_i2c, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C); // SDA_OLED
    gpio_set_function(15, GPIO_FUNC_I2C); // SCL_OLED
    gpio_pull_up(14);
    gpio_pull_up(15);
    
    // Delay para estabilização
    sleep_ms(100);
    
    // Inicializa o display SSD1306
    ssd1306_init();
    
    // Limpa o display completamente
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
    
    // Texto simples centralizado
    ssd1306_draw_string(buffer, 20, 20, "Bem-vindo ao");
    ssd1306_draw_string(buffer, 30, 35, "BioCooler");
    
    render_on_display(buffer, &area);
}

void display_mensagem_central(const char* msg) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    
    // Quebrar a mensagem em linhas se necessário
    char* lines[3] = {NULL, NULL, NULL};
    char temp_msg[64];
    strncpy(temp_msg, msg, sizeof(temp_msg));
    
    // Quebrar por \n se existir
    char* token = strtok(temp_msg, "\n");
    int line_count = 0;
    
    while (token != NULL && line_count < 3) {
        lines[line_count++] = token;
        token = strtok(NULL, "\n");
    }
    
    // Calcular posições Y para centralizar verticalmente
    int start_y;
    if (line_count == 1) {
        start_y = 28;
    } else if (line_count == 2) {
        start_y = 22;
    } else {
        start_y = 16;
    }
    
    // Desenhar cada linha centralizada
    for (int i = 0; i < line_count; i++) {
        if (lines[i] != NULL) {
            int text_width = strlen(lines[i]) * 6;
            int x_pos = (ssd1306_width - text_width) / 2;
            ssd1306_draw_string(buffer, x_pos, start_y + i * 12, lines[i]);
        }
    }
    
    render_on_display(buffer, &area);
}

// Mostra até 4 linhas de texto
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

void display_tela_senha() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    
    ssd1306_draw_string(buffer, 10, 10, "Digite a senha:");
    
    // Exibe asteriscos para cada dígito digitado
    char senha_display[5] = "";
    for (int i = 0; i < get_senha_pos(); i++) {
        senha_display[i] = '*';
    }
    senha_display[get_senha_pos()] = '\0';
    
    ssd1306_draw_string(buffer, 10, 30, senha_display);
    ssd1306_draw_string(buffer, 10, 50, "B para confirmar");
    
    render_on_display(buffer, &area);
}

void display_menu_principal(int menu_idx) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
    
    // Ajustar scroll para manter o item selecionado visível
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
        "Controle Tampa",
        "Config. Tempo",
        "Travar/Destravar",
        "Finalizar"
    };
    
    // Mostrar apenas as opções visíveis
    for (int i = 0; i < max_visible_items; i++) {
        int item_idx = scroll_offset + i;
        if (item_idx >= 8) break;
        
        int y_pos = 16 + i * 12;
        
        if (menu_idx == item_idx) {
            ssd1306_draw_filled_circle(buffer, 4, y_pos + 4, 3);
        }
        
        ssd1306_draw_string(buffer, 10, y_pos, opcoes[item_idx]);
    }
    
    // Indicador de scroll
    if (scroll_offset > 0) {
        ssd1306_draw_string(buffer, 120, 0, "^");
    }
    if (scroll_offset + max_visible_items < 8) {
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
    
    // Centralizar o tempo
    char tempo_str[32];
    snprintf(tempo_str, sizeof(tempo_str), "%02lu:%02lu", horas, minutos);
    
    int text_width = strlen(tempo_str) * 6;
    int x_pos = (ssd1306_width - text_width) / 2;
    
    ssd1306_draw_string(buffer, x_pos, 20, tempo_str);
    
    // Indicador de campo editável
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
    
    char status[20];
    switch (estado_caixa) {
        case 0: strcpy(status, "Status: Aberta"); break;
        case 1: strcpy(status, "Status: Fechada"); break;
        case 2: strcpy(status, "Status: Travada"); break;
        default: strcpy(status, "Status: Desconhecido"); break;
    }
    
    ssd1306_draw_string(buffer, 0, 16, status);
    
    if (estado_caixa == 1) {
        ssd1306_draw_string(buffer, 0, 32, "B para travar");
    } else if (estado_caixa == 2) {
        ssd1306_draw_string(buffer, 0, 32, "B para destravar");
    }
    
    ssd1306_draw_string(buffer, 0, 48, "A para voltar");
    
    render_on_display(buffer, &area);
}

void display_temperatura(float temperatura) {
    char temp_str[22];
    snprintf(temp_str, sizeof(temp_str), "Temp: %.1f C", temperatura);
    display_linhas("Temperatura", temp_str, "", "< A para voltar");
}

void display_pressao(float pressao) {
    char press_str[22];
    snprintf(press_str, sizeof(press_str), "Press: %.1f hPa", pressao / 100.0);
    display_linhas("Pressao", press_str, "", "< A para voltar");
}

void display_luminosidade(uint16_t lux) {
    char lux_str[22];
    snprintf(lux_str, sizeof(lux_str), "Lum: %d lux", lux);
    display_linhas("BH1750", lux_str, "", "< A para voltar");
}

void display_acelerometro(int16_t acc_x, int16_t acc_y, int16_t acc_z) {
    char acel_str[22];
    snprintf(acel_str, sizeof(acel_str), "X:%d Y:%d Z:%d", acc_x, acc_y, acc_z);
    display_linhas("Acelerometro", acel_str, "", "< A para voltar");
}

void display_finalizando() {
    display_linhas("Finalizando...", "", "", "");
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