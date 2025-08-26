#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/bmp280.h"
#include "inc/mpu6050_i2c.h"
#include "inc/luminosidade.h"
#include "inc/wifi_dashboard.h"
#include "inc/bmp280_defs.h"
#include "firmware/wifi/wifi-connect.h"
#include "firmware/dashboard/dashboard.h"

// Declarações antecipadas de funções para o LED RGB
void init_led_rgb();
void led_rgb_set(bool r, bool g, bool b);
void led_vermelho(bool estado);
void led_verde(bool estado);
void led_azul(bool estado);
void led_amarelo(bool estado);
void led_ciano(bool estado);
void led_magenta(bool estado);
void led_branco(bool estado);
void led_desligado(void);


// Adicione estas variáveis globais para controle de scroll
int scroll_offset = 0;
int max_visible_items = 4; // Quantas opções cabem na tela



// Declarações antecipadas de funções para o buzzer
void init_buzzer();
void buzzer_on(void);
void buzzer_off();
void buzzer_beep(uint32_t duracao_ms);
void buzzer_alarme(uint32_t duracao_ms);
void buzzer_alerta_aceleracao(float aceleracao, uint32_t duracao_ms);
void buzzer_alerta_temperatura(float temperatura, uint32_t duracao_ms);
void buzzer_alerta_tempo(uint32_t tempo_restante_ms, uint32_t duracao_ms);

// Declarações antecipadas de funções para alertas
// void verificar_alertas(float temperatura, float accel_x, float accel_y, float accel_z); // Função removida
void verificar_alerta_tempo(void);

// === Definições de hardware ===
#define SDA_OLED 14
#define SCL_OLED 15
#define BUTTON_A 5
#define BUTTON_B 6
#define JOY_X 26
#define JOY_Y 27
#define DEADZONE 1000

// === Definições do teclado matricial ===
#define KEYPAD_ROW1 8  // GPIO08
#define KEYPAD_ROW2 9  // GPIO09
#define KEYPAD_ROW3 4  // GPIO04
#define KEYPAD_ROW4 20 // GPIO20
#define KEYPAD_COL1 19 // GPIO19
#define KEYPAD_COL2 18 // GPIO18
#define KEYPAD_COL3 16 // GPIO16
#define KEYPAD_COL4 17 // GPIO17

// === Definição do servomotor ===
#define SERVO_PIN 22   // Pino para o servomotor

// === Definição do buzzer ===
#define BUZZER_PIN 10  // GPIO10 para o buzzer

// === Definição do LED RGB ===
#define LED_R 11       // GPIO11 para o LED vermelho
#define LED_G 12       // GPIO12 para o LED verde
#define LED_B 13       // GPIO13 para o LED azul

// === Instâncias de I2C ===
static i2c_inst_t *oled_i2c   = i2c1;
static i2c_inst_t *sensor_i2c = i2c0;

// === Estados possíveis do menu ===
typedef enum {
    TELA_BEM_VINDO,
    TELA_SENHA,
    MENU_PRINCIPAL,
    MENU_TEMPERATURA,
    MENU_PRESSAO,
    MENU_LUMINOSIDADE,
    MENU_ACELEROMETRO,
    MENU_CONTROLE_TAMPA,
    MENU_CONFIG_TEMPO,
    MENU_CONFIG_TEMPO2,
    MENU_CONFIG_TEMPO_ALT,
    MENU_TRAVAR_DESTRAVAR,
    MENU_FINALIZAR
} EstadoMenu;


#include "inc/estado_caixa.h"

EstadoMenu estado = TELA_BEM_VINDO;
EstadoCaixa estado_caixa = CAIXA_FECHADA;

// === Variáveis para controle de tempo de entrega ===
uint32_t tempo_entrega_ms = 0;      // Tempo configurado para entrega em milissegundos
uint32_t tempo_inicio_ms = 0;       // Momento em que a contagem iniciou
uint32_t tempo_restante_ms = 0;     // Tempo restante para entrega em milissegundos
uint32_t alerta_tempo_min = 10;     // Minutos antes do fim para acionar alerta (padrão: 10 min)
bool alarme_tempo_ativo = false;    // Indica se o alarme de tempo está ativo
bool alerta_tempo_disparado = false; // Indica se o alerta de tempo já foi disparado

// === Variáveis para controle de senha ===
// char senha_digitada[5] = "";        // Senha digitada pelo usuário - já definida acima
// char senha_correta[5] = "1234";     // Senha correta para destravar - já definida acima
// uint8_t senha_pos = 0;              // Posição atual na senha - já definida acima

// === Matriz do teclado matricial ===
char keypad_keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// === Variáveis para controle de alertas ===
#define TEMP_MAXIMA 30.0           // Temperatura máxima em °C
#define ACEL_MAXIMA 1.5            // Aceleração máxima em g (1g = 9.8 m/s²)
bool alerta_temp_ativo = false;     // Indica se o alerta de temperatura está ativo
bool alerta_acel_ativo = false;     // Indica se o alerta de aceleração está ativo
bool alerta_tempo_ativo = false;    // Indica se o alerta de tempo está ativo

// Declarações de funções para o LED RGB já foram feitas anteriormente

// === Variáveis para controle de data e hora ===
char data_hora_atual[24];          // String com data e hora atual (formato: YYYY-MM-DD HH:MM:SS)

// === Variáveis para controle do LED RGB ===
bool led_r_estado = false;
bool led_g_estado = false;
bool led_b_estado = false;

// Declaração de função externa do dashboard Wi-Fi
extern void wifi_dashboard_update_box_status(bool is_open);

// Função para obter a data e hora atual (simulada no Pico)
void obter_data_hora_atual(char *buffer, size_t buffer_size) {
    // No Pico não temos RTC, então vamos simular com base no tempo desde o boot
    uint32_t ms_desde_boot = to_ms_since_boot(get_absolute_time());
   
    // Convertendo para segundos, minutos, horas, etc.
    uint32_t segundos = (ms_desde_boot / 1000) % 60;
    uint32_t minutos = (ms_desde_boot / (1000 * 60)) % 60;
    uint32_t horas = (ms_desde_boot / (1000 * 60 * 60)) % 24;
   
    // Data fixa para simulação (2023-06-15)
    snprintf(buffer, buffer_size, "2023-06-15 %02lu:%02lu:%02lu", horas, minutos, segundos);
}

// Função para verificar alertas removida - usando a implementação mais completa abaixo

// Função para verificar alerta de tempo
void verificar_alerta_tempo(void) {
    // Verificar alerta de tempo (se contagem estiver ativa)
    if (tempo_entrega_ms > 0 && tempo_inicio_ms > 0) {
        uint32_t agora = to_ms_since_boot(get_absolute_time());
        tempo_restante_ms = (agora < tempo_inicio_ms + tempo_entrega_ms) ?
                           (tempo_inicio_ms + tempo_entrega_ms - agora) : 0;
       
        // Verificar se está no momento de acionar o alerta de tempo
        uint32_t limite_alerta = alerta_tempo_min * 60 * 1000; // Converter minutos para ms
       
        if (tempo_restante_ms > 0 && tempo_restante_ms <= limite_alerta && !alerta_tempo_disparado) {
            alerta_tempo_disparado = true;
            alerta_tempo_ativo = true;
            led_azul(true);      // LED azul para alerta de tempo
            buzzer_alarme(1);    // 1 beep para tempo
        } else if (tempo_restante_ms == 0 && !alarme_tempo_ativo) {
            // Tempo esgotado, acionar alarme final
            alarme_tempo_ativo = true;
            led_magenta(true);   // LED magenta para tempo esgotado
            buzzer_alarme(5);    // 5 beeps para tempo esgotado
        } else if (tempo_restante_ms > limite_alerta && alerta_tempo_disparado) {
            // Resetar o alerta se o tempo for reconfigurado
            alerta_tempo_disparado = false;
            alerta_tempo_ativo = false;
            if (!alerta_temp_ativo && !alerta_acel_ativo) {
                led_desligado();
            } else if (alerta_temp_ativo) {
                led_vermelho(true);  // Restaura LED vermelho se temperatura ainda alta
            } else if (alerta_acel_ativo) {
                led_amarelo(true);   // Restaura LED amarelo se aceleração ainda alta
            }
        }
    }
}
void atualizar_data_hora() {
    // No Pico não temos RTC, então vamos simular a data e hora
    // baseado no tempo desde o boot
    static uint32_t ultimo_segundo = 0;
    static uint32_t segundos_totais = 0;
   
    uint32_t agora = to_ms_since_boot(get_absolute_time());
   
    // Incrementa os segundos a cada 1000ms
    if (agora - ultimo_segundo >= 1000) {
        segundos_totais++;
        ultimo_segundo = agora;
    }
   
    // Calcula horas, minutos e segundos
    uint32_t segundos = segundos_totais % 60;
    uint32_t minutos = (segundos_totais / 60) % 60;
    uint32_t horas = (segundos_totais / 3600) % 24;
   
    // Simula uma data fixa (2023-06-15) + horas desde o boot
    snprintf(data_hora_atual, sizeof(data_hora_atual), "2023-06-15 %02lu:%02lu:%02lu", horas, minutos, segundos);
}

// Função para verificar alertas de temperatura, aceleração e tempo
// Implementação removida - usando a função atualizar_alertas mais completa

// Função para atualizar os dados dos sensores no dashboard
void atualizar_dashboard(float temperatura, float pressao, uint16_t luminosidade,
                        int16_t accel_x, int16_t accel_y, int16_t accel_z,
                        bool caixa_aberta) {
    // Atualizar data e hora
    atualizar_data_hora();
   
    // Verificar alertas
    // verificar_alertas(temperatura, accel_x, accel_y, accel_z); // Função removida
   
    // Criar estrutura com os dados dos sensores
    sensor_data_t sensor_data = {
        .temperatura = temperatura,
        .pressao = pressao,
        .luminosidade = luminosidade,
        .aceleracao_x = accel_x,
        .aceleracao_y = accel_y,
        .aceleracao_z = accel_z,
        .caixa_aberta = caixa_aberta,
        .tempo_entrega_ms = tempo_entrega_ms,
        .tempo_restante_ms = tempo_restante_ms,
        .alerta_tempo_min = alerta_tempo_min,
        .alerta_temp_ativo = alerta_temp_ativo,
        .alerta_acel_ativo = alerta_acel_ativo,
        .alarme_tempo_ativo = alarme_tempo_ativo,
        .alerta_tempo_ativo = alerta_tempo_ativo,
        .data_hora = {0}
    };
   
    // Copiar a string de data e hora
    strncpy(sensor_data.data_hora, data_hora_atual, sizeof(sensor_data.data_hora) - 1);
   
    // Atualizar os dados no dashboard
    dashboard_update_sensor_data(&sensor_data);
}

// Declarações de funções para o servomotor
void servo_travar(void);
void servo_destravar(void);

// Declarações de funções para o buzzer
void init_buzzer(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep(uint32_t duracao_ms);
void buzzer_alarme(uint32_t duracao_ms);

// Variáveis para o controle de senha

// === Variáveis para o controle de senha ===
char senha_correta[5] = "1234";
char senha_digitada[5] = "";
int senha_pos = 0;

// === Matriz do teclado ===
// char keypad_keys[4][4] = {  // já definida acima
//     {'1', '2', '3', 'A'},
//     {'4', '5', '6', 'B'},
//     {'7', '8', '9', 'C'},
//     {'*', '0', '#', 'D'}
// };

// === Variáveis para o servomotor ===
// uint slice_num;  // já definida na função init_servo

// ===================================================================
//                FUNÇÕES GRÁFICAS PARA O DISPLAY
// ===================================================================

// Desenha um pequeno coração na tela (ícone 8x8)
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

// Mostra até 4 linhas de texto
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

// Círculo preenchido (usado para indicar seleção no menu)
void ssd1306_draw_filled_circle(uint8_t *ssd, int x_center, int y_center, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                ssd1306_set_pixel(ssd, x_center + x, y_center + y, true);
            }
        }
    }
}

// Tela de boas-vindas com coração
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
 //monitor serial
 // Função para mostrar os dados dos sensores no monitor serial
void mostrar_dados_sensores(const sensor_data_t *data) {
    if (!data) return;
    printf("=== Dados dos Sensores ===\n");
    printf("Temperatura: %.2f °C\n", data->temperatura);
    printf("Pressao: %.2f hPa\n", data->pressao);
    printf("Luminosidade: %u lux\n", data->luminosidade);
    printf("Aceleracao X: %d\n", data->aceleracao_x);
    printf("Aceleracao Y: %d\n", data->aceleracao_y);
    printf("Aceleracao Z: %d\n", data->aceleracao_z);
    printf("=========================\n");
}



// Mensagem centralizada
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

// Tela de senha
void display_tela_senha() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};

    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);

    ssd1306_draw_string(buffer, 10, 10, "Digite a senha:");
   
    // Exibe asteriscos para cada dígito digitado
    char senha_display[5] = "";
    for (int i = 0; i < senha_pos; i++) {
        senha_display[i] = '*';
    }
    senha_display[senha_pos] = '\0';
   
    ssd1306_draw_string(buffer, 10, 30, senha_display);
    ssd1306_draw_string(buffer, 10, 50, "B para confirmar");

    render_on_display(buffer, &area);
}

// Tela de controle da tampa
void display_controle_tampa() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};

    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);

    ssd1306_draw_string(buffer, 0, 0, "Controle da Tampa");
   
    char status[20];
    switch (estado_caixa) {
        case CAIXA_ABERTA:
            strcpy(status, "Status: Aberta");
            break;
        case CAIXA_FECHADA:
            strcpy(status, "Status: Fechada");
            break;
        case CAIXA_FECHADA_TRAVADA:
            strcpy(status, "Status: Travada");
            break;
    }
   
    ssd1306_draw_string(buffer, 0, 16, status);
   
    if (estado_caixa == CAIXA_FECHADA) {
        ssd1306_draw_string(buffer, 0, 32, "B para travar");
    } else if (estado_caixa == CAIXA_FECHADA_TRAVADA) {
        ssd1306_draw_string(buffer, 0, 32, "B para destravar");
    }
   
    ssd1306_draw_string(buffer, 0, 48, "A para voltar");

    render_on_display(buffer, &area);
}


// Adicione esta função para limpar o display completamente
void clear_display() {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
   
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof(buffer));
    render_on_display(buffer, &area);
}


// Menu principal com destaque na opção atual
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
       
        int y_pos = 16 + i * 12; // Aumentado de 8 para 12 pixels de espaçamento
       
        if (menu_idx == item_idx) {
            ssd1306_draw_filled_circle(buffer, 4, y_pos + 4, 3); // Marcador
        }
       
        ssd1306_draw_string(buffer, 10, y_pos, opcoes[item_idx]);
    }
   
    // Indicador de scroll (setinha para cima/baixo se houver mais itens)
    if (scroll_offset > 0) {
        ssd1306_draw_string(buffer, 120, 0, "^");
    }
    if (scroll_offset + max_visible_items < 8) {
        ssd1306_draw_string(buffer, 120, 56, "v");
    }
   
    render_on_display(buffer, &area);
}

// Tela de configuração de tempo
void display_config_tempo(uint32_t horas, uint32_t minutos, bool editando_horas) {
    uint8_t buffer[ssd1306_buffer_length];
    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
   
    calculate_render_area_buffer_length(&area);
    memset(buffer, 0, sizeof buffer);
   
    ssd1306_draw_string(buffer, 0, 0, "Config. Tempo Entrega");
   
    // Centralizar o tempo
    char tempo_str[32];
    snprintf(tempo_str, sizeof(tempo_str), "%02lu:%02lu", horas, minutos);
   
    // Calcular posição centralizada
    int text_width = strlen(tempo_str) * 6; // Aproximadamente 6 pixels por caractere
    int x_pos = (ssd1306_width - text_width) / 2;
   
    ssd1306_draw_string(buffer, x_pos, 20, tempo_str);
   
    // Indicador de campo editável com melhor visualização
    if (editando_horas) {
        ssd1306_draw_string(buffer, x_pos, 30, "^^");
    } else {
        ssd1306_draw_string(buffer, x_pos + 25, 30, "^^");
    }
   
    // Instruções em linhas separadas com melhor espaçamento
    ssd1306_draw_string(buffer, 0, 40, "Joystick: Ajustar");
    ssd1306_draw_string(buffer, 0, 50, "A: Campo  B: Confirmar");
   
    render_on_display(buffer, &area);
}

// Tela de configuração de tempo de alerta
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


// ===================================================================
//                      FUNÇÕES DE ENTRADA
// ===================================================================

// Inicialização do display OLED
void init_display() {
    // Inicializa o I2C
    i2c_init(oled_i2c, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);
   
    // Delay para estabilização
    sleep_ms(100);
   
    // Inicializa o display SSD1306
    ssd1306_init();
   
    // Limpa o display completamente
    clear_display();
   
    printf("Display OLED inicializado\n");
}

// Botão A com debounce
bool btn_a_pressionado() {
    static uint32_t ultimo_ms = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    if (!gpio_get(BUTTON_A)) { // botão pressionado (nível baixo)
        if (agora - ultimo_ms > 50) { // debounce
            ultimo_ms = agora;
            return true;
        }
    }
    return false;
}

// Botão B com debounce
bool btn_b_pressionado() {
    static uint32_t ultimo_ms = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    if (!gpio_get(BUTTON_B)) { // botão pressionado (nível baixo)
        if (agora - ultimo_ms > 50) { // debounce
            ultimo_ms = agora;
            return true;
        }
    }
    return false;
}

// Função para travar ou destravar a tampa da caixa
void travar_tampa_da_caixa() {
    // Ler o valor atual do sensor de luminosidade
    float lux = get_luminosidade();
   
    // Verificar se a caixa está fechada (luminosidade baixa)
    if (lux < 10) {
        // Se a caixa estiver fechada e não travada, travar
        if (estado_caixa == CAIXA_FECHADA) {
            servo_travar();
            estado_caixa = CAIXA_FECHADA_TRAVADA;
            display_mensagem_central("Tampa travada");
            // Atualizar status no dashboard Wi-Fi
            wifi_dashboard_update_box_status(false);
            sleep_ms(1000);
        }
        // Se a caixa estiver fechada e travada, destravar
        else if (estado_caixa == CAIXA_FECHADA_TRAVADA) {
            servo_destravar();
            estado_caixa = CAIXA_FECHADA;
            display_mensagem_central("Tampa destravada");
            // Atualizar status no dashboard Wi-Fi
            wifi_dashboard_update_box_status(false);
            sleep_ms(1000);
        }
    } else {
        // Se a caixa estiver aberta, não pode travar
        if (estado_caixa != CAIXA_ABERTA) {
            estado_caixa = CAIXA_ABERTA;
            // Atualizar status no dashboard Wi-Fi
            wifi_dashboard_update_box_status(true);
        }
        display_mensagem_central("Caixa aberta!\nFeche para travar");
        sleep_ms(1000);
    }
}

// Detecta direção do joystick (cima/baixo)
int8_t joystick_direcao() {
    static int8_t ultimo_dir = 0;
    adc_select_input(1);        // eixo Y do joystick
    uint16_t y = adc_read();

    int8_t dir = 0;
    if (y < DEADZONE) dir = 1;       // para cima
    else if (y > 4000) dir = -1;     // para baixo

    if (dir != ultimo_dir && dir != 0) {
        ultimo_dir = dir;
        return dir;
    } else if (dir == 0) {
        ultimo_dir = 0;
    }
    return 0;
}

// Inicializa o teclado matricial
void init_keypad() {
    // Configura as linhas como saída
    gpio_init(KEYPAD_ROW1);
    gpio_init(KEYPAD_ROW2);
    gpio_init(KEYPAD_ROW3);
    gpio_init(KEYPAD_ROW4);
    gpio_set_dir(KEYPAD_ROW1, GPIO_OUT);
    gpio_set_dir(KEYPAD_ROW2, GPIO_OUT);
    gpio_set_dir(KEYPAD_ROW3, GPIO_OUT);
    gpio_set_dir(KEYPAD_ROW4, GPIO_OUT);
    gpio_put(KEYPAD_ROW1, 1);
    gpio_put(KEYPAD_ROW4, 1);
   
    // Configura as colunas como entrada com pull-up
    gpio_init(KEYPAD_COL1);
    gpio_init(KEYPAD_COL2);
    gpio_init(KEYPAD_COL3);
    gpio_init(KEYPAD_COL4);
    gpio_set_dir(KEYPAD_COL1, GPIO_IN);
    gpio_set_dir(KEYPAD_COL2, GPIO_IN);
    gpio_set_dir(KEYPAD_COL3, GPIO_IN);
    gpio_set_dir(KEYPAD_COL4, GPIO_IN);
    gpio_pull_up(KEYPAD_COL1);
    gpio_pull_up(KEYPAD_COL2);
    gpio_pull_up(KEYPAD_COL3);
    gpio_pull_up(KEYPAD_COL4);
   
    printf("Teclado matricial inicializado\n");
}

// Lê o teclado matricial e retorna a tecla pressionada ou '\0' se nenhuma tecla for pressionada
char read_keypad() {
    static uint32_t last_key_time = 0;
    static char last_key = '\0';
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
   
    // Debounce para evitar leituras múltiplas da mesma tecla
    if (current_time - last_key_time < 250) { // Aumentado para 250ms para melhor debounce
        return '\0';
    }
   
    // Array com os pinos das linhas
    uint gpio_rows[4] = {KEYPAD_ROW1, KEYPAD_ROW2, KEYPAD_ROW3, KEYPAD_ROW4};
    // Array com os pinos das colunas
    uint gpio_cols[4] = {KEYPAD_COL1, KEYPAD_COL2, KEYPAD_COL3, KEYPAD_COL4};
   
    char key_pressed = '\0';
   
    // Escaneia o teclado
    for (int row = 0; row < 4; row++) {
        // Configura todas as linhas como nível alto (inativas)
        for (int i = 0; i < 4; i++) {
            gpio_put(gpio_rows[i], 1);
        }
       
        // Ativa apenas a linha atual (nível baixo)
        gpio_put(gpio_rows[row], 0);
        sleep_us(300); // Aumentado o delay para melhor estabilização
       
        // Verifica cada coluna
        for (int col = 0; col < 4; col++) {
            // Se a coluna estiver em nível baixo, uma tecla foi pressionada
            if (gpio_get(gpio_cols[col]) == 0) {
                // Verificação adicional para evitar leituras falsas
                sleep_us(50); // Aumentado delay para confirmação
                if (gpio_get(gpio_cols[col]) == 0) { // Confirma a leitura
                    // Verificação dupla para garantir que não é um ruído
                    sleep_us(50);
                    if (gpio_get(gpio_cols[col]) == 0) {
                        key_pressed = keypad_keys[row][col];
                       
                        // Restaura todas as linhas para nível alto
                        for (int i = 0; i < 4; i++) {
                            gpio_put(gpio_rows[i], 1);
                        }
                       
                        // Emite beep para feedback tátil apenas se for uma tecla diferente da última
                        if (key_pressed != last_key) {
                            buzzer_beep(50);
                        }
                       
                        // Aguarda a liberação da tecla para evitar repetições
                        int timeout = 0;
                        while (gpio_get(gpio_cols[col]) == 0 && timeout < 200) { // Aumentado timeout
                            sleep_ms(2); // Aumentado intervalo
                            timeout++;
                        }
                       
                        // Atualiza o tempo da última tecla apenas após a liberação
                        last_key_time = to_ms_since_boot(get_absolute_time());
                        last_key = key_pressed;
                       
                        return key_pressed;
                    }
                }
            }
        }
    }
   
    // Restaura todas as linhas para nível alto
    for (int i = 0; i < 4; i++) {
        gpio_put(gpio_rows[i], 1);
    }
   
    return '\0'; // Nenhuma tecla pressionada
}

// Inicializa o servomotor
void init_servo() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
   
    // Configuração do PWM para 50Hz (20ms de período)
    pwm_set_clkdiv(slice_num, 100);
    pwm_set_wrap(slice_num, 25000);
    pwm_set_enabled(slice_num, true);
   
    // Posição inicial (fechado)
    pwm_set_gpio_level(SERVO_PIN, 1838); // 90 graus
}

// Controla o servomotor para travar (90 graus)
void servo_travar() {
    pwm_set_gpio_level(SERVO_PIN, 1838); // 90 graus
}

// Controla o servomotor para destravar (0 graus)
void servo_destravar() {
    pwm_set_gpio_level(SERVO_PIN, 625); // 0 graus
}

// Inicializa o buzzer
void init_buzzer() {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0); // Inicialmente desligado
}

// Liga o buzzer
void buzzer_on() {
    gpio_put(BUZZER_PIN, 1);
}

// Desliga o buzzer
void buzzer_off() {
    gpio_put(BUZZER_PIN, 0);
}

// Faz o buzzer emitir um beep por um tempo determinado
void buzzer_beep(uint32_t duracao_ms) {
    // Verificar se é um beep de feedback para o teclado ou uma condição de alerta
    // Apenas emitir som se for uma condição de alerta (temperatura alta, aceleração brusca, tempo esgotado)
    if (alerta_temp_ativo || alerta_acel_ativo || alarme_tempo_ativo) {
        buzzer_on();
        sleep_ms(duracao_ms);
        buzzer_off();
    }
}

// Faz o buzzer emitir um alarme (beeps rápidos) por um tempo determinado
void buzzer_alarme(uint32_t duracao_ms) {
    // Verificar se é uma condição de alerta
    // Apenas emitir som se for uma condição de alerta (temperatura alta, aceleração brusca, tempo esgotado)
    if (alerta_temp_ativo || alerta_acel_ativo || alarme_tempo_ativo) {
        uint32_t start_time = to_ms_since_boot(get_absolute_time());
        uint32_t current_time;
       
        // Padrão de som diferente para cada tipo de alerta
        if (alerta_acel_ativo) {
            // Padrão para movimento brusco: beeps rápidos e intensos
            do {
                buzzer_on();
                sleep_ms(50);  // Beeps mais curtos
                buzzer_off();
                sleep_ms(50);  // Pausas mais curtas
                current_time = to_ms_since_boot(get_absolute_time());
            } while (current_time - start_time < duracao_ms);
        }
        else if (alerta_temp_ativo) {
            // Padrão para temperatura alta: beeps médios
            do {
                buzzer_on();
                sleep_ms(150);
                buzzer_off();
                sleep_ms(100);
                current_time = to_ms_since_boot(get_absolute_time());
            } while (current_time - start_time < duracao_ms);
        }
        else {
            // Padrão padrão para outros alertas
            do {
                buzzer_on();
                sleep_ms(100);
                buzzer_off();
                sleep_ms(100);
                current_time = to_ms_since_boot(get_absolute_time());
            } while (current_time - start_time < duracao_ms);
        }
    }
}

// Função específica para alerta de aceleração (movimentos bruscos)
// Emite um padrão sonoro que varia conforme a intensidade do movimento
void buzzer_alerta_aceleracao(float aceleracao, uint32_t duracao_ms) {
    if (!alerta_acel_ativo) return; // Só emite som se o alerta estiver ativo
   
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t current_time;
   
    // Calcular a intensidade relativa do movimento (quanto maior a aceleração, mais intenso o som)
    float intensidade = (aceleracao - ACEL_MAXIMA) / ACEL_MAXIMA;
    if (intensidade < 0.1f) intensidade = 0.1f;
    if (intensidade > 2.0f) intensidade = 2.0f;
   
    // Ajustar os tempos de beep e pausa com base na intensidade
    uint32_t tempo_beep = (uint32_t)(80.0f / intensidade);
    uint32_t tempo_pausa = (uint32_t)(40.0f / intensidade);
   
    // Limitar os tempos mínimos para evitar beeps muito curtos
    if (tempo_beep < 30) tempo_beep = 30;
    if (tempo_pausa < 20) tempo_pausa = 20;
   
    // Emitir o padrão sonoro
    do {
        buzzer_on();
        sleep_ms(tempo_beep);
        buzzer_off();
        sleep_ms(tempo_pausa);
        current_time = to_ms_since_boot(get_absolute_time());
    } while (current_time - start_time < duracao_ms);
}

// Função específica para alerta de temperatura alta (>30°C)
// Emite um padrão sonoro que varia conforme a intensidade da temperatura
void buzzer_alerta_temperatura(float temperatura, uint32_t duracao_ms) {
    if (!alerta_temp_ativo) return; // Só emite som se o alerta estiver ativo
   
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t current_time;
   
    // Calcular a intensidade relativa da temperatura (quanto maior a temperatura, mais intenso o som)
    float intensidade = (temperatura - TEMP_MAXIMA) / 10.0f; // 10°C acima do máximo seria intensidade 1.0
    if (intensidade < 0.1f) intensidade = 0.1f;
    if (intensidade > 1.0f) intensidade = 1.0f;
   
    // Ajustar os tempos de beep e pausa com base na intensidade
    uint32_t tempo_beep = (uint32_t)(150.0f);
    uint32_t tempo_pausa = (uint32_t)(100.0f / intensidade);
   
    // Limitar os tempos mínimos para evitar beeps muito curtos
    if (tempo_pausa < 50) tempo_pausa = 50;
   
    // Emitir o padrão sonoro - beeps mais longos para temperatura
    do {
        buzzer_on();
        sleep_ms(tempo_beep);
        buzzer_off();
        sleep_ms(tempo_pausa);
        current_time = to_ms_since_boot(get_absolute_time());
    } while (current_time - start_time < duracao_ms);
}

// Função específica para alerta de tempo restante
// Emite um padrão sonoro que varia conforme o tempo restante
void buzzer_alerta_tempo(uint32_t tempo_restante_ms, uint32_t duracao_ms) {
    if (!alarme_tempo_ativo) return; // Só emite som se o alarme de tempo estiver ativo
   
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t current_time;
   
    uint32_t tempo_alerta_ms = alerta_tempo_min * 60 * 1000; // Converter minutos para ms
   
    if (tempo_restante_ms == 0) {
        // Padrão para tempo esgotado: beeps longos e pausas curtas
        uint32_t tempo_beep = 500;
        uint32_t tempo_pausa = 200;
       
        // Emitir o padrão sonoro para tempo esgotado
        do {
            buzzer_on();
            sleep_ms(tempo_beep);
            buzzer_off();
            sleep_ms(tempo_pausa);
            current_time = to_ms_since_boot(get_absolute_time());
        } while (current_time - start_time < duracao_ms);
    } else if (tempo_restante_ms <= tempo_alerta_ms) {
        // Calcular a urgência baseada no tempo restante
        // Quanto menor o tempo restante, mais rápido e intenso o padrão sonoro
        float percentual_restante = (float)tempo_restante_ms / (float)tempo_alerta_ms;
       
        // Ajustar os tempos de beep e pausa baseados na urgência
        uint32_t tempo_beep = 100 + (uint32_t)(percentual_restante * 200);
        uint32_t tempo_pausa = 100 + (uint32_t)(percentual_restante * 300);
       
        // Emitir o padrão sonoro para alerta de tempo restante
        do {
            buzzer_on();
            sleep_ms(tempo_beep);
            buzzer_off();
            sleep_ms(tempo_pausa);
            current_time = to_ms_since_boot(get_absolute_time());
        } while (current_time - start_time < duracao_ms);
    }
}

// Atualiza o estado dos alertas com base nos valores dos sensores
// Esta função substitui a implementação anterior de verificar_alertas
void atualizar_alertas(float temperatura, float aceleracao_total) {
    bool alerta_acionado = false;
   
    // Verificar alerta de temperatura
    static uint32_t ultimo_alerta_temp = 0;
    uint32_t agora_temp = to_ms_since_boot(get_absolute_time());
   
    if (temperatura > TEMP_MAXIMA && !alerta_temp_ativo) {
        alerta_temp_ativo = true;
        alerta_acionado = true;
        ultimo_alerta_temp = agora_temp;
        led_vermelho(true); // LED vermelho para temperatura alta
       
        // Calcular duração do alarme baseado na intensidade da temperatura
        uint32_t duracao_alarme = 2000 + (uint32_t)((temperatura - TEMP_MAXIMA) * 200);
        duracao_alarme = (duracao_alarme > 5000) ? 5000 : duracao_alarme; // Limitar a 5 segundos
       
        // Emitir alarme com padrão específico para temperatura alta usando a função especializada
        buzzer_alerta_temperatura(temperatura, duracao_alarme);
       
        // Registrar no log
        printf("ALERTA: Temperatura alta detectada! Temperatura: %.2f °C\n", temperatura);
    } else if (temperatura <= TEMP_MAXIMA && alerta_temp_ativo) {
        // Desativar alerta se a temperatura voltar ao normal
        if (agora_temp - ultimo_alerta_temp > 3000) { // Esperar 3 segundos para evitar oscilações
            alerta_temp_ativo = false;
        }
    }
   
    // Verificar alerta de aceleração (movimentos bruscos)
    static uint32_t ultimo_alerta_acel = 0;
    static uint32_t contador_acel_alta = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());
   
    // Detectar movimento brusco (aceleração acima do limite)
    if (aceleracao_total > ACEL_MAXIMA) {
        contador_acel_alta++;
       
        // Acionar alerta se não estiver ativo e se já passou tempo suficiente desde o último alerta
        // ou se a aceleração for muito alta (movimento muito brusco)
        if (!alerta_acel_ativo &&
            ((agora - ultimo_alerta_acel > 5000) || (aceleracao_total > ACEL_MAXIMA * 1.5))) {
            alerta_acel_ativo = true;
            alerta_acionado = true;
            ultimo_alerta_acel = agora;
            led_amarelo(true); // LED amarelo para movimento brusco
           
            // Padrão de alerta sonoro específico para movimento brusco
             // Sequência de beeps rápidos com duração proporcional à intensidade do movimento
             uint32_t duracao_alarme = 1500 + (uint32_t)((aceleracao_total - ACEL_MAXIMA) * 1000);
             duracao_alarme = (duracao_alarme > 4000) ? 4000 : duracao_alarme; // Limitar a 4 segundos
             
             // Emitir alarme com padrão específico para movimento brusco usando a função especializada
             buzzer_alerta_aceleracao(aceleracao_total, duracao_alarme);
           
            // Registrar no log (poderia ser expandido para registrar em arquivo)
            printf("ALERTA: Movimento brusco detectado! Aceleração: %.2f g\n", aceleracao_total);
        }
    } else {
        // Resetar contador se a aceleração estiver abaixo do limite
        contador_acel_alta = 0;
       
        // Desativar alerta se estiver ativo e a aceleração voltar ao normal
        if (alerta_acel_ativo && (agora - ultimo_alerta_acel > 2000)) {
            alerta_acel_ativo = false;
        }
    }
   
    // Verificar alerta de tempo restante
    if (alarme_tempo_ativo && tempo_entrega_ms > 0) {
        uint32_t tempo_alerta_ms = alerta_tempo_min * 60 * 1000; // Converter minutos para ms
        static uint32_t ultimo_alerta_tempo = 0;
        uint32_t agora_tempo = to_ms_since_boot(get_absolute_time());
       
        // Se o tempo restante for menor que o tempo de alerta
        if (tempo_restante_ms > 0 && tempo_restante_ms <= tempo_alerta_ms) {
            alerta_acionado = true;
            led_azul(true); // LED azul para alerta de tempo
           
            // Emitir alerta sonoro a cada 10 segundos quando estiver na zona de alerta
            if (agora_tempo - ultimo_alerta_tempo > 10000) { // 10 segundos entre alertas
                ultimo_alerta_tempo = agora_tempo;
               
                // Calcular duração do alarme baseado na urgência do tempo restante
                uint32_t duracao_alarme = 1000 + (uint32_t)((1.0f - ((float)tempo_restante_ms / (float)tempo_alerta_ms)) * 2000);
                duracao_alarme = (duracao_alarme > 3000) ? 3000 : duracao_alarme; // Limitar a 3 segundos
               
                // Emitir alarme com padrão específico para tempo restante
                buzzer_alerta_tempo(tempo_restante_ms, duracao_alarme);
               
                // Registrar no log
                printf("ALERTA: Tempo restante baixo! Restante: %lu ms\n", tempo_restante_ms);
            }
        }
       
        // Se o tempo acabou completamente
        if (tempo_restante_ms == 0) {
            alerta_acionado = true;
            led_magenta(true); // LED magenta para tempo esgotado
           
            // Emitir alarme final apenas uma vez
            if (!ultimo_alerta_tempo || agora_tempo - ultimo_alerta_tempo > 5000) { // Evitar repetição rápida
                ultimo_alerta_tempo = agora_tempo;
                buzzer_alerta_tempo(0, 5000); // Alarme de 5 segundos para tempo esgotado
                printf("ALERTA: Tempo esgotado!\n");
            }
           
            // Desativa o alarme após soar
            if (agora_tempo - ultimo_alerta_tempo > 6000) { // Esperar o alarme terminar
                alarme_tempo_ativo = false;
            }
        }
    }
   
    // Se nenhum alerta estiver ativo, retorna o LED para o estado normal
    if (!alerta_temp_ativo && !alerta_acel_ativo && !alerta_acionado) {
        if (alarme_tempo_ativo) {
            led_verde(true); // Verde quando timer está ativo sem alertas
        } else {
            led_desligado(); // Desliga o LED quando não há alertas ou timer
        }
    }
}

// Inicializa os LEDs RGB
void init_led_rgb() {
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
    led_desligado(); // Inicialmente desligado
}

// Configura os LEDs RGB
void led_rgb_set(bool r, bool g, bool b) {
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
    led_r_estado = r;
    led_g_estado = g;
    led_b_estado = b;
}

// Funções para controlar cores específicas
void led_vermelho(bool estado) {
    led_rgb_set(estado, false, false);
}

void led_verde(bool estado) {
    led_rgb_set(false, estado, false);
}

void led_azul(bool estado) {
    led_rgb_set(false, false, estado);
}

void led_amarelo(bool estado) {
    led_rgb_set(estado, estado, false);
}

void led_ciano(bool estado) {
    led_rgb_set(false, estado, estado);
}

void led_magenta(bool estado) {
    led_rgb_set(estado, false, estado);
}

void led_branco(bool estado) {
    led_rgb_set(estado, estado, estado);
}

void led_desligado() {
    led_rgb_set(false, false, false);
}


// ===================================================================
//                      INICIALIZAÇÃO DE HARDWARE
// ===================================================================

// Inicializa sensor BH1750 (luminosidade) - função já definida em luminosidade.c

// Inicialização geral do hardware
void init_hw_without_display() {
    // Botão A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
   
    // Botão B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Joystick
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);

    // I2C sensores (mas não inicializa display ainda)
    i2c_init(sensor_i2c, 100000);
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    // Sensores extras
    mpu6050_setup_i2c();
    mpu6050_reset();
    bmp280_init();
    init_luminosidade();
   
    // Teclado matricial
    init_keypad();
   
    // Servomotor
    init_servo();
   
    // Buzzer
    init_buzzer();
   
    // LED RGB
    init_led_rgb();
}


// ===================================================================
//                             MAIN
// ===================================================================
int main() {

    stdio_init_all();
    init_display();
    clear_display();
    display_bem_vindo();
   
    // Agora inicializar o resto do hardware em background
    init_hw_without_display(); // Você precisará criar esta função
   
    sleep_ms(800);
    clear_display();
    display_mensagem_central("Iniciando...");
   
    // Inicializar Wi-Fi usando o novo módulo
    if (init_wifi()) {
        printf("Wi-Fi inicializado com sucesso\n");
       
        // Inicializar o dashboard
        if (dashboard_init()) {
            printf("Dashboard inicializado com sucesso\n");
        } else {
            printf("Falha ao inicializar o dashboard\n");
        }
    } else {
        printf("Falha ao inicializar o Wi-Fi\n");
    }
   
    int menu_idx = 0;
    uint32_t ultimo_envio_dashboard = 0;
    const uint32_t intervalo_envio_dashboard = 3000; // Enviar dados a cada 3 segundos

    while (1) {
        // Verificar se é hora de enviar dados para o dashboard
        uint32_t agora = to_ms_since_boot(get_absolute_time());
       
        // Atualizar o tempo restante se o contador estiver ativo
        if (alarme_tempo_ativo && tempo_entrega_ms > 0 && tempo_inicio_ms > 0) {
            tempo_restante_ms = (agora < tempo_inicio_ms + tempo_entrega_ms) ?
                               (tempo_inicio_ms + tempo_entrega_ms - agora) : 0;
           
            // Verificar se está no momento de acionar o alerta de tempo
            verificar_alerta_tempo();
        }
       
        if (agora - ultimo_envio_dashboard >= intervalo_envio_dashboard) {
            // Ler dados dos sensores
            float temperatura = 0.0f;
            float pressao = 0.0f;
            float luminosidade = get_luminosidade();
            int16_t acc[3], gyr[3], temp;
            mpu6050_read_raw(acc, gyr, &temp);
           
            // Calcular aceleração total (magnitude do vetor)
            float accel_total = sqrtf((float)(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2])) / 16384.0f;
           
            // Verificar alertas (temperatura, aceleração e tempo)
            atualizar_alertas(temperatura, accel_total);
           
            // Verificar se a caixa está aberta ou fechada
            bool caixa_aberta = (estado_caixa == CAIXA_ABERTA);
           
            // Obter data e hora atual
            char data_hora[24];
            obter_data_hora_atual(data_hora, sizeof(data_hora));
           
            // Atualizar o dashboard
            sensor_data_t sensor_data = {
                .temperatura = temperatura,
                .pressao = pressao,
                .luminosidade = luminosidade,
                .aceleracao_x = acc[0],
                .aceleracao_y = acc[1],
                .aceleracao_z = acc[2],
                .caixa_aberta = caixa_aberta,
                .tempo_entrega_ms = tempo_entrega_ms,
                .tempo_restante_ms = tempo_restante_ms,
                .alerta_tempo_min = alerta_tempo_min,
                .alerta_temp_ativo = alerta_temp_ativo,
                .alerta_acel_ativo = alerta_acel_ativo,
                .alarme_tempo_ativo = alarme_tempo_ativo,
                .alerta_tempo_ativo = alerta_tempo_ativo,
            };
           
            // Copiar a data e hora
            strncpy(sensor_data.data_hora, data_hora, sizeof(sensor_data.data_hora) - 1);
            sensor_data.data_hora[sizeof(sensor_data.data_hora) - 1] = '\0';
           
            // Enviar dados para o dashboard
            dashboard_update_sensor_data(&sensor_data);
           
            ultimo_envio_dashboard = agora;
        }
        switch (estado) {
// === Tela inicial ===
case TELA_BEM_VINDO:
    clear_display();
    display_bem_vindo();
    sleep_ms(800); // Tempo reduzido para ver a mensagem
   
    // Mostrar "Iniciando..." mas não esperar aqui
    clear_display();
    display_mensagem_central("Iniciando...");
   
    // Mudar imediatamente para o menu principal
    // A inicialização continua em background
    estado = MENU_PRINCIPAL;
    menu_idx = 0;
    scroll_offset = 0;
    break;
               
            // === Tela de senha ===
            case TELA_SENHA: {
                display_tela_senha();
               
                // Ler teclado matricial
                char tecla = read_keypad();
                if (tecla != '\0' && tecla >= '0' && tecla <= '9' && senha_pos < 4) {
                    senha_digitada[senha_pos++] = tecla;
                    display_tela_senha(); // Atualiza a tela com o novo asterisco
                    sleep_ms(200); // Pequeno delay para evitar múltiplas leituras da mesma tecla
                }
               
                // Verificar senha quando pressionar o botão B
                if (btn_b_pressionado()) {
                    if (senha_pos == 4 && strncmp(senha_digitada, senha_correta, 4) == 0) {
                        // Senha correta
                        display_mensagem_central("Senha correta!");
                        sleep_ms(1000);
                       
                        // Se a caixa estiver travada, destravar
                        if (estado_caixa == CAIXA_FECHADA_TRAVADA) {
                            servo_destravar();
                            estado_caixa = CAIXA_FECHADA;
                            display_mensagem_central("Tampa destravada");
                            sleep_ms(1000);
                        }
                       
                        display_mensagem_central("A iniciar transporte");
                        sleep_ms(1000);
                        estado = MENU_PRINCIPAL;
                    } else {
                        // Senha incorreta
                        display_mensagem_central("Senha incorreta!");
                        sleep_ms(1000);
                       
                        // Resetar senha digitada
                        memset(senha_digitada, 0, sizeof(senha_digitada));
                        senha_pos = 0;
                    }
                }
               
                sleep_ms(100);
                break;
            }

            // === Menu principal ===
            case MENU_PRINCIPAL: {
                display_menu_principal(menu_idx);

                int8_t dir = joystick_direcao();
if (dir == 1) { // cima
    menu_idx = (menu_idx - 1 + 8) % 8;
} else if (dir == -1) { // baixo
    menu_idx = (menu_idx + 1) % 8;
}
                // Verificar se a caixa está fechada e não travada para oferecer opção de travar com botão B
                float lux = get_luminosidade();
                if (lux == 0 && estado_caixa == CAIXA_FECHADA && btn_b_pressionado()) {
                    // Travar a caixa diretamente com o botão B se iluminação for zero
                    servo_travar();
                    estado_caixa = CAIXA_FECHADA_TRAVADA;
                    display_mensagem_central("Tampa travada");
                    // Atualizar status no dashboard Wi-Fi
                    wifi_dashboard_update_box_status(false);
                    sleep_ms(1000);
                    break;
                }

                // Verificar se a caixa está travada e o usuário quer destravar com o teclado
                if (estado_caixa == CAIXA_FECHADA_TRAVADA) {
                    char tecla = read_keypad();
                    if (tecla != '\0') {
                        // Aceitar apenas dígitos numéricos se ainda não completou a senha
                        if (tecla >= '0' && tecla <= '9' && senha_pos < 4) {
                            senha_digitada[senha_pos++] = tecla;
                            buzzer_beep(50); // Feedback sonoro ao digitar
                        }
                        // Tecla * para limpar a senha
                        else if (tecla == '*') {
                            // Resetar senha digitada
                            memset(senha_digitada, 0, sizeof(senha_digitada));
                            senha_pos = 0;
                            buzzer_beep(100); // Feedback sonoro ao limpar
                        }
                       
                        // Atualizar display com a senha atual
                        char senha_display[10] = "";
                        for (int i = 0; i < senha_pos; i++) {
                            strcat(senha_display, "*");
                        }
                       
                        char senha_linha[22];
                        snprintf(senha_linha, sizeof(senha_linha), "Senha: %s", senha_display);
                        display_linhas("Destravar", "Digite a senha", senha_linha, "A:Voltar B:Confirmar *:Limpar");
                       
                        sleep_ms(200);
                        break; // Continuar no menu principal após exibir a mensagem
                    }
                   
                    // Verificar se o botão A foi pressionado para cancelar
                    if (btn_a_pressionado()) {
                        display_mensagem_central("Operação cancelada");
                        led_amarelo(true); // Feedback visual - LED amarelo
                        buzzer_beep(100); // Feedback sonoro
                        sleep_ms(1000);
                        led_desligado();
                       
                        // Resetar senha digitada
                        memset(senha_digitada, 0, sizeof(senha_digitada));
                        senha_pos = 0;
                       
                        break; // Continuar no menu principal
                    }
                   
                    // Verificar senha quando pressionar o botão B
                    if (btn_b_pressionado() && senha_pos > 0) {
                        if (senha_pos == 4 && strncmp(senha_digitada, senha_correta, 4) == 0) {
                            // Senha correta
                            display_mensagem_central("Senha correta!");
                            led_verde(true); // Feedback visual - LED verde
                            buzzer_beep(200); // Feedback sonoro positivo
                            sleep_ms(1000);
                            led_desligado();
                           
                            servo_destravar();
                            estado_caixa = CAIXA_FECHADA;
                            display_mensagem_central("Tampa destravada");
                            // Atualizar status no dashboard Wi-Fi
                            wifi_dashboard_update_box_status(false);
                            sleep_ms(1000);
                           
                            // Resetar senha digitada
                            memset(senha_digitada, 0, sizeof(senha_digitada));
                            senha_pos = 0;
                           
                            break; // Continuar no menu principal
                        } else {
                            // Senha incorreta ou incompleta
                            display_mensagem_central("Senha incorreta!");
                            led_vermelho(true); // Feedback visual - LED vermelho
                            buzzer_alarme(500); // Feedback sonoro negativo
                            sleep_ms(1000);
                            led_desligado();
                           
                            // Resetar senha digitada
                            memset(senha_digitada, 0, sizeof(senha_digitada));
                            senha_pos = 0;
                           
                            break; // Continuar no menu principal
                        }
                    }
                }

                if (btn_a_pressionado()) {
                    switch (menu_idx) {
                        case 0: estado = MENU_TEMPERATURA; break;
                        case 1: estado = MENU_PRESSAO; break;
                        case 2: estado = MENU_LUMINOSIDADE; break;
                        case 3: estado = MENU_ACELEROMETRO; break;
                        case 4: estado = MENU_CONTROLE_TAMPA; break;
                        case 5: estado = MENU_CONFIG_TEMPO; break;
                        case 6: estado = MENU_TRAVAR_DESTRAVAR; break;
                        case 7: estado = MENU_FINALIZAR; break;
                    }
                }
                sleep_ms(150);
                break;
            }

            // === Menu Temperatura ===
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

            // === Menu Pressão ===
            case MENU_PRESSAO: {
                while (1) {
                    sensors_t dados = bmp280_get_all(0x76);
                    char u[22];
                    snprintf(u, sizeof(u), "Press: %.1f hPa", dados.pressure / 100.0);

                    display_linhas("Pressao", u, "", "< A para voltar");

                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    sleep_ms(200);
                }
                break;
            }

            // === Menu Configuração de Tempo ===
            case MENU_CONFIG_TEMPO: {
                uint32_t tempo_horas = tempo_entrega_ms / 3600000;
                uint32_t tempo_minutos = (tempo_entrega_ms % 3600000) / 60000;
                bool editando_horas = true;
               
                while (1) {
                    // Usar a função de display específica para configuração de tempo
                    display_config_tempo(tempo_horas, tempo_minutos, editando_horas);
                   
                    // Ler joystick para aumentar/diminuir valores
                    int8_t dir = joystick_direcao();
                    if (dir == 1) { // cima
                        if (editando_horas) {
                            if (tempo_horas < 99) tempo_horas++;
                        } else {
                            if (tempo_minutos < 59) tempo_minutos++;
                        }
                    } else if (dir == -1) { // baixo
                        if (editando_horas) {
                            if (tempo_horas > 0) tempo_horas--;
                        } else {
                            if (tempo_minutos > 0) tempo_minutos--;
                        }
                    }
                   
                    // Botão A para alternar entre horas e minutos
                    if (btn_a_pressionado()) {
                        editando_horas = !editando_horas;
                        buzzer_beep(100); // Feedback sonoro
                    }
                   
                    // Botão B para confirmar e ir para configuração de alerta
                    if (btn_b_pressionado()) {
                        // Salvar o tempo configurado
                        tempo_entrega_ms = (tempo_horas * 3600000) + (tempo_minutos * 60000);
                       
                        // Se o tempo for maior que zero, vá para a configuração de alerta
                        if (tempo_entrega_ms > 0) {
                            estado = MENU_CONFIG_TEMPO2;
                            buzzer_beep(200); // Feedback sonoro
                        } else {
                            // Se o tempo for zero, desative o timer e volte ao menu principal
                            alarme_tempo_ativo = false;
                            alerta_tempo_disparado = false;
                            led_desligado();
                            display_mensagem_central("Timer desativado");
                            sleep_ms(1000);
                            estado = MENU_PRINCIPAL;
                        }
                        break;
                    }
                   
                    sleep_ms(150);
                }
                break;
            }
           
            // === Menu Configuração de Tempo de Alerta ===
            case MENU_CONFIG_TEMPO2: {
               
                while (1) {
                    // Usar a função de display específica para configuração de alerta
                    display_config_tempo_alerta(alerta_tempo_min);
                   
                    // Ler joystick para aumentar/diminuir valores
                    int8_t dir = joystick_direcao();
                    if (dir == 1) { // cima
                        if (alerta_tempo_min < 60) alerta_tempo_min++;
                    } else if (dir == -1) { // baixo
                        if (alerta_tempo_min > 1) alerta_tempo_min--;
                    }
                   
                    // Botão B para confirmar e iniciar a contagem
                    if (btn_b_pressionado()) {
                       
                        // Iniciar a contagem
                        tempo_inicio_ms = to_ms_since_boot(get_absolute_time());
                        alarme_tempo_ativo = true;
                        alerta_tempo_disparado = false;
                        led_verde(true); // Indicação visual de que o timer está ativo
                       
                        display_mensagem_central("Tempo configurado!\nContagem iniciada");
                        buzzer_beep(200); // Feedback sonoro
                        sleep_ms(1000);
                       
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                   
                    // Botão A para voltar sem salvar
                    if (btn_a_pressionado()) {
                        estado = MENU_CONFIG_TEMPO;
                        break;
                    }
                   
                    sleep_ms(150);
                }
                break;
            }

            // === Menu Travar/Destravar ===
            case MENU_TRAVAR_DESTRAVAR: {
                // Verificar se a caixa está aberta
                float lux = get_luminosidade();
               
                if (lux > 0) { // Caixa aberta
                    display_linhas("Travar/Destravar", "Caixa aberta", "Feche a caixa primeiro", "A:Voltar");
                } else { // Caixa fechada (iluminação igual a zero)
                    // Se a caixa estiver travada, solicitar senha para destravar
                    if (estado_caixa == CAIXA_FECHADA_TRAVADA) {
                        // Inicialmente mostrar a tela de senha
                        char senha_linha[22];
                        char senha_display[10] = "";
                       
                        // Mostrar asteriscos para cada dígito digitado
                        for (int i = 0; i < senha_pos; i++) {
                            strcat(senha_display, "*");
                        }
                       
                        snprintf(senha_linha, sizeof(senha_linha), "Senha: %s", senha_display);
                        display_linhas("Travar/Destravar", "Caixa travada", senha_linha, "A:Voltar B:Confirmar *:Limpar");
                       
                        // Ler teclado matricial
                        char tecla = read_keypad();
                        if (tecla != '\0') {
                            // Aceitar apenas dígitos numéricos se ainda não completou a senha
                            if (tecla >= '0' && tecla <= '9' && senha_pos < 4) {
                                senha_digitada[senha_pos++] = tecla;
                                buzzer_beep(50); // Feedback sonoro ao digitar
                               
                                // Atualizar imediatamente o display com o novo asterisco
                                senha_display[0] = '\0'; // Limpar string
                                for (int i = 0; i < senha_pos; i++) {
                                    strcat(senha_display, "*");
                                }
                                snprintf(senha_linha, sizeof(senha_linha), "Senha: %s", senha_display);
                                display_linhas("Travar/Destravar", "Caixa travada", senha_linha, "A:Voltar B:Confirmar *:Limpar");
                            }
                            // Tecla * para limpar a senha
                            else if (tecla == '*') {
                                // Resetar senha digitada
                                memset(senha_digitada, 0, sizeof(senha_digitada));
                                senha_pos = 0;
                                buzzer_beep(100); // Feedback sonoro ao limpar
                               
                                // Atualizar imediatamente o display com a senha limpa
                                display_linhas("Travar/Destravar", "Caixa travada", "Senha: ", "A:Voltar B:Confirmar *:Limpar");
                            }
                           
                            sleep_ms(200); // Pequeno delay para evitar múltiplas leituras da mesma tecla
                        }
                       
                        // Verificar se o botão A foi pressionado para cancelar
                        if (btn_a_pressionado()) {
                            display_mensagem_central("Operação cancelada");
                            led_amarelo(true); // Feedback visual - LED amarelo
                            buzzer_beep(100); // Feedback sonoro
                            sleep_ms(1000);
                            led_desligado();
                           
                            // Resetar senha digitada
                            memset(senha_digitada, 0, sizeof(senha_digitada));
                            senha_pos = 0;
                           
                            estado = MENU_PRINCIPAL;
                            break;
                        }
                       
                        // Verificar senha quando pressionar o botão B
                        if (btn_b_pressionado()) {
                            if (senha_pos == 4 && strncmp(senha_digitada, senha_correta, 4) == 0) {
                                // Senha correta
                                display_mensagem_central("Senha correta!");
                                led_verde(true); // Feedback visual - LED verde
                                buzzer_beep(200); // Feedback sonoro positivo
                                sleep_ms(1000);
                                led_desligado();
                               
                                servo_destravar();
                                estado_caixa = CAIXA_FECHADA;
                                display_mensagem_central("Tampa destravada");
                                // Atualizar status no dashboard Wi-Fi
                                wifi_dashboard_update_box_status(false);
                                sleep_ms(1000);
                               
                                // Resetar senha digitada
                                memset(senha_digitada, 0, sizeof(senha_digitada));
                                senha_pos = 0;
                               
                                estado = MENU_PRINCIPAL;
                                break;
                            } else if (senha_pos < 4) {
                                // Senha incompleta
                                display_mensagem_central("Senha incompleta!\nDigite 4 dígitos");
                                led_amarelo(true); // Feedback visual - LED amarelo
                                buzzer_beep(100); // Feedback sonoro de alerta
                                sleep_ms(1500);
                                led_desligado();
                               
                                // Voltar para a tela de entrada de senha
                                char senha_linha[22];
                                char senha_display[10] = "";
                                for (int i = 0; i < senha_pos; i++) {
                                    strcat(senha_display, "*");
                                }
                                snprintf(senha_linha, sizeof(senha_linha), "Senha: %s", senha_display);
                                display_linhas("Travar/Destravar", "Caixa travada", senha_linha, "A:Voltar B:Confirmar *:Limpar");
                            } else {
                                // Senha incorreta
                                display_mensagem_central("Senha incorreta!");
                                led_vermelho(true); // Feedback visual - LED vermelho
                                buzzer_alarme(500); // Feedback sonoro negativo
                                sleep_ms(1000);
                                led_desligado();
                               
                                // Resetar senha digitada após erro
                                memset(senha_digitada, 0, sizeof(senha_digitada));
                                senha_pos = 0;
                               
                                // Voltar para a tela de entrada de senha
                                display_linhas("Travar/Destravar", "Caixa travada", "Senha: ", "A:Voltar B:Confirmar *:Limpar");
                                senha_pos = 0;
                            }
                        }
                    } else { // Caixa fechada mas não travada
                        display_linhas("Travar/Destravar", "Caixa fechada", "Deseja travar?", "A:Voltar B:Travar");
                       
                        if (btn_b_pressionado()) {
                            servo_travar();
                            estado_caixa = CAIXA_FECHADA_TRAVADA;
                            display_mensagem_central("Tampa travada");
                            // Atualizar status no dashboard Wi-Fi
                            wifi_dashboard_update_box_status(false);
                            // Não emite beep para feedback sonoro
                            sleep_ms(1000);
                            estado = MENU_PRINCIPAL;
                            break;
                        }
                    }
                }
               
                if (btn_a_pressionado()) {
                    estado = MENU_PRINCIPAL;
                    break;
                }
               
                sleep_ms(150);
                break;
            }

            // === Menu Finalizar ===
            case MENU_LUMINOSIDADE: {
                while (1) {
                    uint16_t lux = ler_lux();
                    char luxstr[22];
                    snprintf(luxstr, sizeof(luxstr), "Lum: %d lux", lux);

                    display_linhas("BH1750", luxstr, "", "< A para voltar");

                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    sleep_ms(200);
                }
                break;
            }

            // === Menu Acelerômetro ===
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
           
            // === Menu Configuração de Tempo (Alternativo) ===
            case MENU_CONFIG_TEMPO_ALT: {
                uint8_t submenu_idx = 0; // 0: horas, 1: minutos, 2: alerta, 3: salvar
                uint8_t horas = tempo_entrega_ms / (3600 * 1000);
                uint8_t minutos = (tempo_entrega_ms % (3600 * 1000)) / (60 * 1000);
                uint8_t alerta_min = alerta_tempo_min;
                bool configuracao_salva = false;
               
                while (1) {
                    uint8_t buffer[ssd1306_buffer_length];
                    struct render_area area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
                   
                    calculate_render_area_buffer_length(&area);
                    memset(buffer, 0, sizeof buffer);
                   
                    ssd1306_draw_string(buffer, 0, 0, "Config. Tempo Entrega");
                   
                    char str_horas[20], str_minutos[20], str_alerta[30], str_salvar[20];
                    snprintf(str_horas, sizeof(str_horas), "Horas: %02d", horas);
                    snprintf(str_minutos, sizeof(str_minutos), "Minutos: %02d", minutos);
                    snprintf(str_alerta, sizeof(str_alerta), "Alerta: %02d min antes", alerta_min);
                    snprintf(str_salvar, sizeof(str_salvar), "Salvar e Ativar");
                   
                    // Desenha marcador na opção atual
                    ssd1306_draw_filled_circle(buffer, 4, 20 + submenu_idx * 10, 3);
                   
                    ssd1306_draw_string(buffer, 10, 20, str_horas);
                    ssd1306_draw_string(buffer, 10, 30, str_minutos);
                    ssd1306_draw_string(buffer, 10, 40, str_alerta);
                    ssd1306_draw_string(buffer, 10, 50, str_salvar);
                   
                    if (configuracao_salva) {
                        ssd1306_draw_string(buffer, 10, 60, "Timer ativado!");
                    }
                   
                    render_on_display(buffer, &area);
                   
                    // Navegação com joystick
                    int8_t dir = joystick_direcao();
                    if (dir == 1) { // cima
                        submenu_idx = (submenu_idx + 3) % 4;
                    } else if (dir == -1) { // baixo
                        submenu_idx = (submenu_idx + 1) % 4;
                    } else if (dir == 2) { // direita
                        // Aumentar valor
                        switch (submenu_idx) {
                            case 0: // horas
                                horas = (horas + 1) % 100; // máximo 99 horas
                                break;
                            case 1: // minutos
                                minutos = (minutos + 1) % 60;
                                break;
                            case 2: // alerta
                                alerta_min = (alerta_min + 1) % 61; // máximo 60 minutos
                                break;
                        }
                    } else if (dir == -2) { // esquerda
                        // Diminuir valor
                        switch (submenu_idx) {
                            case 0: // horas
                                horas = (horas + 99) % 100; // máximo 99 horas
                                break;
                            case 1: // minutos
                                minutos = (minutos + 59) % 60;
                                break;
                            case 2: // alerta
                                alerta_min = (alerta_min + 60) % 61; // máximo 60 minutos
                                break;
                        }
                    }
                   
                    // Botão A para voltar
                    if (btn_a_pressionado()) {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                   
                    // Botão B para confirmar/salvar
                    if (btn_b_pressionado()) {
                        if (submenu_idx == 3) { // Opção Salvar
                            // Calcular tempo total em ms
                            tempo_entrega_ms = (horas * 3600 + minutos * 60) * 1000;
                            alerta_tempo_min = alerta_min;
                           
                            if (tempo_entrega_ms > 0) {
                                // Iniciar contagem
                                tempo_inicio_ms = to_ms_since_boot(get_absolute_time());
                                alarme_tempo_ativo = true;
                                configuracao_salva = true;
                                led_verde(true); // LED verde indica timer ativo
                                buzzer_beep(200); // Beep curto para confirmar
                            } else {
                                // Desativar contagem se tempo for zero
                                alarme_tempo_ativo = false;
                                led_desligado(); // Desliga o LED
                            }
                        }
                    }
                   
                    sleep_ms(150);
                }
                break;
            }
           
            // === Menu Controle da Tampa ===
            case MENU_CONTROLE_TAMPA: {
                display_controle_tampa();
               
                // Verificar se o botão B foi pressionado para travar/destravar a tampa
                if (btn_b_pressionado()) {
                    travar_tampa_da_caixa();
                    display_controle_tampa();
                }
               
                if (btn_a_pressionado()) estado = MENU_PRINCIPAL;
                sleep_ms(150);
                break;
            }

            // === Finalizar ===
            case MENU_FINALIZAR: {
                display_mensagem_central("Finalizando...");
                sleep_ms(2000);
                display_mensagem_central("Ate logo!");
                sleep_ms(2000);
                estado = TELA_BEM_VINDO;
                break;
            }
        }
    }
}