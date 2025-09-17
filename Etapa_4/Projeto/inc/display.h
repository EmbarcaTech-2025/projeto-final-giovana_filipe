#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "inc/ssd1306.h"

extern int scroll_offset;
extern int max_visible_items;
extern int senha_pos;

typedef enum {
    TELA_BEM_VINDO,
    TELA_INICIAR_TRANSPORTE,
    TELA_SENHA,
    MENU_PRINCIPAL,
    MENU_TEMPERATURA,
    MENU_PRESSAO,
    MENU_LUMINOSIDADE,
    MENU_ACELEROMETRO,
    MENU_HUMIDADE,              // << NOVO
    MENU_CONTROLE_TAMPA,
    MENU_CONFIG_TEMPO,
    MENU_CONFIG_TEMPO2,
    MENU_CONFIG_TEMPO_ALT,
    MENU_TRAVAR_DESTRAVAR,
    MENU_FINALIZAR
} EstadoMenu;

extern int scroll_offset;
extern int max_visible_items;

void init_display(void);
void clear_display(void);

void ssd1306_draw_heart(uint8_t *ssd, int x, int y);
void ssd1306_draw_filled_circle(uint8_t *ssd, int x_center, int y_center, int radius);

void display_bem_vindo(void);
void display_mensagem_central(const char* msg);
void display_tela_senha();
void display_controle_tampa(int estado_caixa);
void display_menu_principal(int menu_idx);
void display_config_tempo(uint32_t horas, uint32_t minutos, bool editando_horas);
void display_config_tempo_alerta(uint32_t minutos_alerta);
void display_linhas(const char* l1, const char* l2, const char* l3, const char* l4);

void display_temperatura(float temperatura);
void display_pressao(float pressao);
void display_luminosidade(uint16_t lux);
void display_acelerometro(int16_t acc_x, int16_t acc_y, int16_t acc_z);
void display_humidade(float umidade, float temperatura); // << NOVO

#endif
