#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/keypad.h"
#include "inc/bmp280.h"
#include "inc/mpu6050_i2c.h"
#include "inc/luminosidade.h"
#include "inc/display.h"
#include "inc/wifi_dashboard.h"
#include "inc/bmp280_defs.h"
#include "inc/servo.h"
#include "inc/aht10.h"
#include "inc/estado_caixa.h"
#include "firmware/wifi/wifi-connect.h"
#include "firmware/dashboard/dashboard.h"


// Declarações antecipadas de funções para o LED RGB
void init_led_rgb();
void led_rgb_set(bool r, bool g, bool b);
void led_vermelho(bool estado);
void led_verde(bool estado);
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
void verificar_alerta_tempo(void);

// === Definições de hardware ===
#define SDA_OLED 14
#define SCL_OLED 15
#define BUTTON_A 5
#define BUTTON_B 6
#define JOY_X 26
#define JOY_Y 27
#define DEADZONE 1000

// === Definição do buzzer ===
#define BUZZER_PIN 10 // GPIO10 para o buzzer

// === Definição do LED RGB ===
#define LED_R 11 // GPIO11 para o LED vermelho
#define LED_G 12 // GPIO12 para o LED verde
#define LED_B 13 // GPIO13 para o LED azul

// === Instâncias de I2C ===
static i2c_inst_t *oled_i2c = i2c1;
static i2c_inst_t *sensor_i2c = i2c0;

EstadoMenu estado = TELA_BEM_VINDO;
EstadoCaixa estado_caixa = CAIXA_FECHADA;

// === Variáveis para controle de tempo de entrega ===
uint32_t tempo_entrega_ms = 0;       // Tempo configurado para entrega em milissegundos
uint32_t tempo_inicio_ms = 0;        // Momento em que a contagem iniciou
uint32_t tempo_restante_ms = 0;      // Tempo restante para entrega em milissegundos
uint32_t alerta_tempo_min = 10;      // Minutos antes do fim para acionar alerta (padrão: 10 min)
bool alarme_tempo_ativo = false;     // Indica se o alarme de tempo está ativo
bool alerta_tempo_disparado = false; // Indica se o alerta de tempo já foi disparado

// === Variáveis para controle de alertas ===
#define TEMP_MAXIMA 30.0         // Temperatura máxima em °C
#define ACEL_MAXIMA 1.5          // Aceleração máxima em g (1g = 9.8 m/s²)
bool alerta_temp_ativo = false;  // Indica se o alerta de temperatura está ativo
bool alerta_acel_ativo = false;  // Indica se o alerta de aceleração está ativo
bool alerta_tempo_ativo = false; // Indica se o alerta de tempo está ativo

// === Variáveis para controle de data e hora ===
char data_hora_atual[24]; // String com data e hora atual (formato: YYYY-MM-DD HH:MM:SS)

// === Variáveis para controle do LED RGB ===
bool led_r_estado = false;
bool led_g_estado = false;
bool led_b_estado = false;

// Declaração de função externa do dashboard Wi-Fi
extern void wifi_dashboard_update_box_status(bool is_open);

// Função para obter a data e hora atual (simulada no Pico)
void obter_data_hora_atual(char *buffer, size_t buffer_size)
{
    // No Pico não temos RTC, então vamos simular com base no tempo desde o boot
    uint32_t ms_desde_boot = to_ms_since_boot(get_absolute_time());

    // Convertendo para segundos, minutos, horas, etc.
    uint32_t segundos = (ms_desde_boot / 1000) % 60;
    uint32_t minutos = (ms_desde_boot / (1000 * 60)) % 60;
    uint32_t horas = (ms_desde_boot / (1000 * 60 * 60)) % 24;

    // Data fixa para simulação (2023-06-15)
    snprintf(buffer, buffer_size, "2023-06-15 %02lu:%02lu:%02lu", horas, minutos, segundos);
}

// Função para verificar alerta de tempo
void verificar_alerta_tempo(void)
{
    // Verificar alerta de tempo (se contagem estiver ativa)
    if (tempo_entrega_ms > 0 && tempo_inicio_ms > 0)
    {
        uint32_t agora = to_ms_since_boot(get_absolute_time());
        tempo_restante_ms = (agora < tempo_inicio_ms + tempo_entrega_ms) ? (tempo_inicio_ms + tempo_entrega_ms - agora) : 0;

        // Verificar se está no momento de acionar o alerta de tempo
        uint32_t limite_alerta = alerta_tempo_min * 60 * 1000; // Converter minutos para ms

        if (tempo_restante_ms > 0 && tempo_restante_ms <= limite_alerta && !alerta_tempo_disparado)
        {
            alerta_tempo_disparado = true;
            alerta_tempo_ativo = true;
            led_vermelho(true);   
            buzzer_alarme(1); // 1 beep para tempo
        }
        else if (tempo_restante_ms == 0 && !alarme_tempo_ativo)
        {
            // Tempo esgotado, acionar alarme final
            alarme_tempo_ativo = true;
            led_vermelho(true); // LED magenta para tempo esgotado
            buzzer_alarme(5);  // 5 beeps para tempo esgotado
        }
        else if (tempo_restante_ms > limite_alerta && alerta_tempo_disparado)
        {
            // Resetar o alerta se o tempo for reconfigurado
            alerta_tempo_disparado = false;
            alerta_tempo_ativo = false;
            if (!alerta_temp_ativo && !alerta_acel_ativo)
            {
                led_desligado();
            }
            else if (alerta_temp_ativo)
            {
                led_vermelho(true); // Restaura LED vermelho se temperatura ainda alta
            }
            else if (alerta_acel_ativo)
            {
                led_vermelho(true); // Restaura LED amarelo se aceleração ainda alta
            }
        }
    }
}
void atualizar_data_hora()
{
    // No Pico não temos RTC, então vamos simular a data e hora
    // baseado no tempo desde o boot
    static uint32_t ultimo_segundo = 0;
    static uint32_t segundos_totais = 0;

    uint32_t agora = to_ms_since_boot(get_absolute_time());

    // Incrementa os segundos a cada 1000ms
    if (agora - ultimo_segundo >= 1000)
    {
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

// Função para atualizar os dados dos sensores no dashboard
void atualizar_dashboard(float temperatura, float pressao, float umidade,
                         uint16_t luminosidade,
                         int16_t accel_x, int16_t accel_y, int16_t accel_z,
                         float aceleracao_total, bool caixa_aberta, const char *led_status)
{
    // Atualizar data e hora (seu código já tem função)
    atualizar_data_hora();

    uint32_t tempo_decorrido_ms = 0;
    if (tempo_inicio_ms > 0) {
        uint32_t agora = to_ms_since_boot(get_absolute_time());
        tempo_decorrido_ms = agora - tempo_inicio_ms;
    }
    sensor_data_t sensor_data = {
        .temperatura = temperatura,
        .pressao = pressao,
        .umidade = umidade,
        .luminosidade = luminosidade,
        .aceleracao_x = accel_x,
        .aceleracao_y = accel_y,
        .aceleracao_z = accel_z,
        .aceleracao_total = aceleracao_total,
        .caixa_aberta = caixa_aberta,
        .tempo_entrega_ms = tempo_entrega_ms,
        .tempo_restante_ms = tempo_restante_ms,
        .alerta_tempo_min = alerta_tempo_min,
        .alerta_temp_ativo = alerta_temp_ativo,
        .alerta_acel_ativo = alerta_acel_ativo,
        .alarme_tempo_ativo = alarme_tempo_ativo,
        .alerta_tempo_ativo = alerta_tempo_ativo,
        .data_hora = {0},
        .led_status = {0},
        .total_registros = 0,
        .tempo_decorrido_ms = tempo_decorrido_ms
    };

    // Copiar a string de data e hora
    strncpy(sensor_data.data_hora, data_hora_atual, sizeof(sensor_data.data_hora) - 1);

    // Copiar led status
    strncpy(sensor_data.led_status, led_status, sizeof(sensor_data.led_status) - 1);

    dashboard_update_sensor_data(&sensor_data);
}


// Declarações de funções para o buzzer
void init_buzzer(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep(uint32_t duracao_ms);
void buzzer_alarme(uint32_t duracao_ms);

// ===================================================================
//                FUNÇÕES GRÁFICAS PARA O DISPLAY
// ===================================================================

// Função para mostrar os dados dos sensores no monitor serial
void mostrar_dados_sensores(const sensor_data_t *data)
{
    if (!data)
        return;
    printf("=== Dados dos Sensores ===\n");
    printf("Temperatura: %.2f °C\n", data->temperatura);
    printf("Humidade: %.2f °C\n", data->umidade);
    printf("Pressao: %.2f hPa\n", data->pressao);
    printf("Luminosidade: %u lux\n", data->luminosidade);
    printf("Aceleracao X: %d\n", data->aceleracao_x);
    printf("Aceleracao Y: %d\n", data->aceleracao_y);
    printf("Aceleracao Z: %d\n", data->aceleracao_z);
    printf("=========================\n");
}

// ===================================================================
//                      FUNÇÕES DE ENTRADA
// ===================================================================

// Botão A com debounce
bool btn_a_pressionado()
{
    static uint32_t ultimo_ms = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    if (!gpio_get(BUTTON_A))
    { // botão pressionado (nível baixo)
        if (agora - ultimo_ms > 50)
        { // debounce
            ultimo_ms = agora;
            return true;
        }
    }
    return false;
}

// Botão B com debounce
bool btn_b_pressionado()
{
    static uint32_t ultimo_ms = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    if (!gpio_get(BUTTON_B))
    { // botão pressionado (nível baixo)
        if (agora - ultimo_ms > 50)
        { // debounce
            ultimo_ms = agora;
            return true;
        }
    }
    return false;
}

// Detecta direção do joystick (cima/baixo)
int8_t joystick_direcao()
{
    static int8_t ultimo_dir = 0;
    adc_select_input(1); // eixo Y do joystick
    uint16_t y = adc_read();

    int8_t dir = 0;
    if (y < DEADZONE)
        dir = 1; // para cima
    else if (y > 4000)
        dir = -1; // para baixo

    if (dir != ultimo_dir && dir != 0)
    {
        ultimo_dir = dir;
        return dir;
    }
    else if (dir == 0)
    {
        ultimo_dir = 0;
    }
    return 0;
}

// Inicializa o buzzer
void init_buzzer()
{
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0); // Inicialmente desligado
}

// Liga o buzzer
void buzzer_on()
{
    gpio_put(BUZZER_PIN, 1);
}

// Desliga o buzzer
void buzzer_off()
{
    gpio_put(BUZZER_PIN, 0);
}

// Faz o buzzer emitir um beep por um tempo determinado
void buzzer_beep(uint32_t duracao_ms)
{
    // Verificar se é um beep de feedback para o teclado ou uma condição de alerta
    // Apenas emitir som se for uma condição de alerta (temperatura alta, aceleração brusca, tempo esgotado)
    if (alerta_temp_ativo || alerta_acel_ativo || alarme_tempo_ativo)
    {
        buzzer_on();
        sleep_ms(duracao_ms);
        buzzer_off();
    }
}

// Faz o buzzer emitir um alarme (beeps rápidos) por um tempo determinado
void buzzer_alarme(uint32_t duracao_ms)
{
    // Verificar se é uma condição de alerta
    // Apenas emitir som se for uma condição de alerta (temperatura alta, aceleração brusca, tempo esgotado)
    if (alerta_temp_ativo || alerta_acel_ativo || alarme_tempo_ativo)
    {
        uint32_t start_time = to_ms_since_boot(get_absolute_time());
        uint32_t current_time;

        // Padrão de som diferente para cada tipo de alerta
        if (alerta_acel_ativo)
        {
            // Padrão para movimento brusco: beeps rápidos e intensos
            do
            {
                buzzer_on();
                sleep_ms(50); // Beeps mais curtos
                buzzer_off();
                sleep_ms(50); // Pausas mais curtas
                current_time = to_ms_since_boot(get_absolute_time());
            } while (current_time - start_time < duracao_ms);
        }
        else if (alerta_temp_ativo)
        {
            // Padrão para temperatura alta: beeps médios
            do
            {
                buzzer_on();
                sleep_ms(150);
                buzzer_off();
                sleep_ms(100);
                current_time = to_ms_since_boot(get_absolute_time());
            } while (current_time - start_time < duracao_ms);
        }
        else
        {
            // Padrão padrão para outros alertas
            do
            {
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
void buzzer_alerta_aceleracao(float aceleracao, uint32_t duracao_ms)
{
    if (!alerta_acel_ativo)
        return; // Só emite som se o alerta estiver ativo

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t current_time;

    // Calcular a intensidade relativa do movimento (quanto maior a aceleração, mais intenso o som)
    float intensidade = (aceleracao - ACEL_MAXIMA) / ACEL_MAXIMA;
    if (intensidade < 0.1f)
        intensidade = 0.1f;
    if (intensidade > 2.0f)
        intensidade = 2.0f;

    // Ajustar os tempos de beep e pausa com base na intensidade
    uint32_t tempo_beep = (uint32_t)(80.0f / intensidade);
    uint32_t tempo_pausa = (uint32_t)(40.0f / intensidade);

    // Limitar os tempos mínimos para evitar beeps muito curtos
    if (tempo_beep < 30)
        tempo_beep = 30;
    if (tempo_pausa < 20)
        tempo_pausa = 20;

    // Emitir o padrão sonoro
    do
    {
        buzzer_on();
        sleep_ms(tempo_beep);
        buzzer_off();
        sleep_ms(tempo_pausa);
        current_time = to_ms_since_boot(get_absolute_time());
    } while (current_time - start_time < duracao_ms);
}

// Função específica para alerta de temperatura alta (>30°C)
// Emite um padrão sonoro que varia conforme a intensidade da temperatura
void buzzer_alerta_temperatura(float temperatura, uint32_t duracao_ms)
{
    if (!alerta_temp_ativo)
        return; // Só emite som se o alerta estiver ativo

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t current_time;

    // Calcular a intensidade relativa da temperatura (quanto maior a temperatura, mais intenso o som)
    float intensidade = (temperatura - TEMP_MAXIMA) / 10.0f; // 10°C acima do máximo seria intensidade 1.0
    if (intensidade < 0.1f)
        intensidade = 0.1f;
    if (intensidade > 1.0f)
        intensidade = 1.0f;

    // Ajustar os tempos de beep e pausa com base na intensidade
    uint32_t tempo_beep = (uint32_t)(150.0f);
    uint32_t tempo_pausa = (uint32_t)(100.0f / intensidade);

    // Limitar os tempos mínimos para evitar beeps muito curtos
    if (tempo_pausa < 50)
        tempo_pausa = 50;

    // Emitir o padrão sonoro - beeps mais longos para temperatura
    do
    {
        buzzer_on();
        sleep_ms(tempo_beep);
        buzzer_off();
        sleep_ms(tempo_pausa);
        current_time = to_ms_since_boot(get_absolute_time());
    } while (current_time - start_time < duracao_ms);
}

// Função específica para alerta de tempo restante
// Emite um padrão sonoro que varia conforme o tempo restante
void buzzer_alerta_tempo(uint32_t tempo_restante_ms, uint32_t duracao_ms)
{
    if (!alarme_tempo_ativo)
        return; // Só emite som se o alarme de tempo estiver ativo

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t current_time;

    uint32_t tempo_alerta_ms = alerta_tempo_min * 60 * 1000; // Converter minutos para ms

    if (tempo_restante_ms == 0)
    {
        // Padrão para tempo esgotado: beeps longos e pausas curtas
        uint32_t tempo_beep = 500;
        uint32_t tempo_pausa = 200;

        // Emitir o padrão sonoro para tempo esgotado
        do
        {
            buzzer_on();
            sleep_ms(tempo_beep);
            buzzer_off();
            sleep_ms(tempo_pausa);
            current_time = to_ms_since_boot(get_absolute_time());
        } while (current_time - start_time < duracao_ms);
    }
    else if (tempo_restante_ms <= tempo_alerta_ms)
    {
        // Calcular a urgência baseada no tempo restante
        // Quanto menor o tempo restante, mais rápido e intenso o padrão sonoro
        float percentual_restante = (float)tempo_restante_ms / (float)tempo_alerta_ms;

        // Ajustar os tempos de beep e pausa baseados na urgência
        uint32_t tempo_beep = 100 + (uint32_t)(percentual_restante * 200);
        uint32_t tempo_pausa = 100 + (uint32_t)(percentual_restante * 300);

        // Emitir o padrão sonoro para alerta de tempo restante
        do
        {
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
void atualizar_alertas(float temperatura, float aceleracao_total)
{
    bool alerta_acionado = false;

    // Verificar alerta de temperatura
    static uint32_t ultimo_alerta_temp = 0;
    uint32_t agora_temp = to_ms_since_boot(get_absolute_time());

    if (temperatura > TEMP_MAXIMA && !alerta_temp_ativo)
    {
        alerta_temp_ativo = true;
        alerta_acionado = true;
        ultimo_alerta_temp = agora_temp;

        // Calcular duração do alarme baseado na intensidade da temperatura
        uint32_t duracao_alarme = 2000 + (uint32_t)((temperatura - TEMP_MAXIMA) * 200);
        duracao_alarme = (duracao_alarme > 5000) ? 5000 : duracao_alarme; // Limitar a 5 segundos

        // Emitir alarme com padrão específico para temperatura alta usando a função especializada
        buzzer_alerta_temperatura(temperatura, duracao_alarme);

        // Registrar no log
        printf("ALERTA: Temperatura alta detectada! Temperatura: %.2f °C\n", temperatura);
    }
    else if (temperatura <= TEMP_MAXIMA && alerta_temp_ativo)
    {
        // Desativar alerta se a temperatura voltar ao normal
        if (agora_temp - ultimo_alerta_temp > 3000)
        { // Esperar 3 segundos para evitar oscilações
            alerta_temp_ativo = false;
        }
    }

    // Verificar alerta de aceleração (movimentos bruscos)
    static uint32_t ultimo_alerta_acel = 0;
    static uint32_t contador_acel_alta = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    // Detectar movimento brusco (aceleração acima do limite)
    if (aceleracao_total > ACEL_MAXIMA)
    {
        contador_acel_alta++;

        // Acionar alerta se não estiver ativo e se já passou tempo suficiente desde o último alerta
        // ou se a aceleração for muito alta (movimento muito brusco)
        if (!alerta_acel_ativo &&
            ((agora - ultimo_alerta_acel > 5000) || (aceleracao_total > ACEL_MAXIMA * 1.5)))
        {
            alerta_acel_ativo = true;
            alerta_acionado = true;
            ultimo_alerta_acel = agora;

            // Padrão de alerta sonoro específico para movimento brusco
            // Sequência de beeps rápidos com duração proporcional à intensidade do movimento
            uint32_t duracao_alarme = 1500 + (uint32_t)((aceleracao_total - ACEL_MAXIMA) * 1000);
            duracao_alarme = (duracao_alarme > 4000) ? 4000 : duracao_alarme; // Limitar a 4 segundos

            // Emitir alarme com padrão específico para movimento brusco usando a função especializada
            buzzer_alerta_aceleracao(aceleracao_total, duracao_alarme);

            // Registrar no log (poderia ser expandido para registrar em arquivo)
            printf("ALERTA: Movimento brusco detectado! Aceleração: %.2f g\n", aceleracao_total);
        }
    }
    else
    {
        // Resetar contador se a aceleração estiver abaixo do limite
        contador_acel_alta = 0;

        // Desativar alerta se estiver ativo e a aceleração voltar ao normal
        if (alerta_acel_ativo && (agora - ultimo_alerta_acel > 2000))
        {
            alerta_acel_ativo = false;
        }
    }

    // Verificar alerta de tempo restante
    if (alarme_tempo_ativo && tempo_entrega_ms > 0)
    {
        uint32_t tempo_alerta_ms = alerta_tempo_min * 60 * 1000; // Converter minutos para ms
        static uint32_t ultimo_alerta_tempo = 0;
        uint32_t agora_tempo = to_ms_since_boot(get_absolute_time());

        // Se o tempo restante for menor que o tempo de alerta
        if (tempo_restante_ms > 0 && tempo_restante_ms <= tempo_alerta_ms)
        {
            alerta_acionado = true;

            // Emitir alerta sonoro a cada 10 segundos quando estiver na zona de alerta
            if (agora_tempo - ultimo_alerta_tempo > 10000)
            { // 10 segundos entre alertas
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
        if (tempo_restante_ms == 0)
        {
            alerta_acionado = true;

            // Emitir alarme final apenas uma vez
            if (!ultimo_alerta_tempo || agora_tempo - ultimo_alerta_tempo > 5000)
            { // Evitar repetição rápida
                ultimo_alerta_tempo = agora_tempo;
                buzzer_alerta_tempo(0, 5000); // Alarme de 5 segundos para tempo esgotado
                printf("ALERTA: Tempo esgotado!\n");
            }

            // Desativa o alarme após soar
            if (agora_tempo - ultimo_alerta_tempo > 6000)
            { // Esperar o alarme terminar
                alarme_tempo_ativo = false;
            }
        }
    }

    // Gerenciamento do LED RGB
    if (alerta_temp_ativo || alerta_acel_ativo || (alarme_tempo_ativo && tempo_restante_ms <= alerta_tempo_min * 60 * 1000)) {
        led_vermelho(true); // Se qualquer alerta estiver ativo, acende o LED vermelho
    } else if (alarme_tempo_ativo) {
        led_verde(true); // Se o timer estiver ativo e não houver outros alertas, acende o LED verde
    } else {
        led_desligado(); // Desliga o LED quando não há alertas ou timer
    }
}

// Inicializa os LEDs RGB
void init_led_rgb()
{
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
    led_desligado(); // Inicialmente desligado
}

// Configura os LEDs RGB
void led_rgb_set(bool r, bool g, bool b)
{
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
    led_r_estado = r;
    led_g_estado = g;
    led_b_estado = b;
}

// Funções para controlar cores específicas
void led_vermelho(bool estado)
{
    led_rgb_set(estado, false, false);
}

void led_verde(bool estado)
{
    led_rgb_set(false, estado, false);
}

void led_branco(bool estado)
{
    led_rgb_set(estado, estado, estado);
}

void led_desligado()
{
    led_rgb_set(false, false, false);
}

// ===================================================================
//                      INICIALIZAÇÃO DE HARDWARE
// ===================================================================

// Inicializa sensor BH1750 (luminosidade) - função já definida em luminosidade.c

// Inicialização geral do hardware
void init_hw_without_display()
{
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
    aht10_init(sensor_i2c);

    // Teclado matricial
    keypad_init();

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
int main()
{
    stdio_init_all();
    printf("[DEBUG] Iniciando main()...\n");
    init_display();
    printf("[DEBUG] Display inicializado\n");
    clear_display();
    display_bem_vindo();
    printf("[DEBUG] Tela de boas-vindas exibida\n");
    // Agora inicializar o resto do hardware em background
    init_hw_without_display(); // Você precisará criar esta função
    printf("[DEBUG] init_hw_without_display() executado\n");

    sleep_ms(800);
    clear_display();
    display_mensagem_central("Iniciando...");
    printf("[DEBUG] Mensagem 'Iniciando...' exibida\n");

    // Inicializar Wi-Fi usando o novo módulo
    if (init_wifi())
    {
        printf("Wi-Fi inicializado com sucesso\n");

        // Inicializar o dashboard
        if (dashboard_init())
        {
            printf("Dashboard inicializado com sucesso\n");
        }
        else
        {
            printf("Falha ao inicializar o dashboard\n");
        }
    }
    else
    {
        printf("Falha ao inicializar o Wi-Fi\n");
    }

    int menu_idx = 0;
    uint32_t ultimo_envio_dashboard = 0;
    const uint32_t intervalo_envio_dashboard = 3000; // Enviar dados a cada 3 segundos

    printf("[DEBUG] Entrando no while(1)\n");
    while (1)
    {
        printf("[DEBUG] Loop while(1), estado = %d\n", estado);
        // Verificar se é hora de enviar dados para o dashboard
        uint32_t agora = to_ms_since_boot(get_absolute_time());

        // Atualizar o tempo restante se o contador estiver ativo
        if (alarme_tempo_ativo && tempo_entrega_ms > 0 && tempo_inicio_ms > 0)
        {
            tempo_restante_ms = (agora < tempo_inicio_ms + tempo_entrega_ms) ? (tempo_inicio_ms + tempo_entrega_ms - agora) : 0;

            // Verificar se está no momento de acionar o alerta de tempo
            verificar_alerta_tempo();
        }

        if (agora - ultimo_envio_dashboard >= intervalo_envio_dashboard)
        {
            // Ler dados dos sensores

            // CORREÇÃO: Ler os dados do BMP280 e atribuir às variáveis
            sensors_t dados_bmp280 = bmp280_get_all(0x76);
            float temperatura = dados_bmp280.temperature;
            float pressao = dados_bmp280.pressure;

            // CORREÇÃO: Ler luminosidade e atribuir à variável
            uint16_t luminosidade = get_luminosidade();

            // Ler dados do MPU6050
            int16_t acc[3], gyr[3], temp;
            mpu6050_read_raw(acc, gyr, &temp);

            // Calcular aceleração total (magnitude do vetor)
            float accel_total = sqrtf((float)(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2])) / 16384.0f;

            // Ler umidade do AHT10
            float umidade = 0.0f;
            float temp_aht = 0.0f;
            if (!aht10_read_data(sensor_i2c, &umidade, &temp_aht)) {
                // Se falhar, mantém valor anterior (ou zero)
                umidade = 0.0f;
            }

            // Verificar alertas (temperatura, aceleração e tempo)
            atualizar_alertas(temperatura, accel_total);

            // Verificar se a caixa está aberta ou fechada
            bool caixa_aberta = (estado_caixa == CAIXA_ABERTA);

            // Obter status do LED para enviar ao dashboard
            const char* led_status = "all_off";
            if (alerta_temp_ativo || alerta_acel_ativo) {
                led_status = "danger";
            } else if (alarme_tempo_ativo) {
                led_status = "normal";
            }

            // Obter data e hora atual
            char data_hora[24];
            obter_data_hora_atual(data_hora, sizeof(data_hora));

            // Atualizar o dashboard
            // Calcular tempo decorrido desde o início do transporte
            uint32_t tempo_decorrido_ms = 0;
            if (tempo_inicio_ms > 0) {
                tempo_decorrido_ms = agora - tempo_inicio_ms;
            }
            sensor_data_t sensor_data = {
                .temperatura = temperatura,
                .pressao = pressao / 100.0, // A pressão precisa ser convertida de Pa para hPa
                .umidade = umidade,
                .luminosidade = luminosidade,
                .aceleracao_x = acc[0],
                .aceleracao_y = acc[1],
                .aceleracao_z = acc[2],
                .aceleracao_total = accel_total, // Novo campo
                .caixa_aberta = caixa_aberta,
                .tempo_entrega_ms = tempo_entrega_ms,
                .tempo_restante_ms = tempo_restante_ms,
                .alerta_tempo_min = alerta_tempo_min,
                .alerta_temp_ativo = alerta_temp_ativo,
                .alerta_acel_ativo = alerta_acel_ativo,
                .alarme_tempo_ativo = alarme_tempo_ativo,
                .alerta_tempo_ativo = alerta_tempo_ativo,
                .tempo_decorrido_ms = tempo_decorrido_ms,
            };

            // Copiar a data e hora
            strncpy(sensor_data.data_hora, data_hora, sizeof(sensor_data.data_hora) - 1);
            sensor_data.data_hora[sizeof(sensor_data.data_hora) - 1] = '\0';

            // Copiar o status do LED
            strncpy(sensor_data.led_status, led_status, sizeof(sensor_data.led_status) - 1);

            // Enviar dados para o dashboard
            dashboard_update_sensor_data(&sensor_data);

            ultimo_envio_dashboard = agora;
        }
        switch (estado)
        {
            

        // === Tela inicial ===
        case TELA_BEM_VINDO:
            clear_display();
            display_bem_vindo();
            sleep_ms(800);

            // Mostrar "Iniciando..." mas não esperar aqui
            clear_display();
            display_mensagem_central("Iniciando...");
            sleep_ms(800);

            // Ir para tela de iniciar transporte
            estado = TELA_INICIAR_TRANSPORTE;
            break;

        // === Tela de iniciar transporte ===
        case TELA_INICIAR_TRANSPORTE:
            clear_display();
            display_mensagem_central("Iniciar\nTransporte\nSim - A");
            // Espera até o usuário apertar o botão A
            while (!btn_a_pressionado()) {
                sleep_ms(50);
            }
            // Inicia o cronômetro apenas se ainda não estiver rodando
            if (tempo_inicio_ms == 0) {
                tempo_inicio_ms = to_ms_since_boot(get_absolute_time());
            }
            tempo_entrega_ms = 0; // Zera o tempo de entrega, pois será um cronômetro
            estado = MENU_PRINCIPAL;
            menu_idx = 0;
            scroll_offset = 0;
            break;
        
        // === Tela de senha ===
        case MENU_CONTROLE_TAMPA:
        {
            char senha_trava[] = "56780";
            char senha_destrava[] = "12340";
            char entrada[10] = "";
            int pos = 0;
            char msg_display[30] = "Digite a senha:";

            while (1)
            {
                // Exibir na tela
                clear_display();
                display_mensagem_central("Controle da Caixa");
                display_linhas(msg_display, entrada, "", "Voltar: A");

                // Exibir asteriscos para a senha
                char asteriscos[10] = "";
                for (int i = 0; i < pos; i++)
                {
                    asteriscos[i] = '*';
                }
                asteriscos[pos] = '\0';
                display_linhas(msg_display, asteriscos, "", "Voltar: A");

                char key = read_keypad();
                if (key)
                {
                    if (key == 'A') // Botão 'A' do teclado para cancelar a entrada
                    {
                        estado = MENU_PRINCIPAL;
                        break;
                    }
                    else if (key == 'C') // Botão 'C' para apagar
                    {
                        if (pos > 0)
                        {
                            pos--;
                            entrada[pos] = '\0';
                        }
                    }
                    else
                    {
                        printf("Tecla: %c\n", key);
                        entrada[pos++] = key;
                        entrada[pos] = '\0';

                        // Verificar senha quando o comprimento atingir o esperado
                        if (pos == strlen(senha_trava))
                        {
                            if (strcmp(entrada, senha_trava) == 0)
                            {
                                servo_travar();
                                estado_caixa = CAIXA_FECHADA;
                                display_mensagem_central("Caixa Trancada!");
                                buzzer_beep(500);
                                sleep_ms(1000);
                                estado = MENU_PRINCIPAL;
                                break;
                            }
                            else if (strcmp(entrada, senha_destrava) == 0)
                            {
                                servo_destravar();
                                estado_caixa = CAIXA_ABERTA;
                                display_mensagem_central("Caixa Destrancada!");
                                buzzer_beep(500);
                                sleep_ms(1000);
                                estado = MENU_PRINCIPAL;
                                break;
                            }
                            else
                            {
                                snprintf(msg_display, sizeof(msg_display), "Senha Incorreta!");
                                buzzer_beep(100);
                                sleep_ms(1000);
                                snprintf(msg_display, sizeof(msg_display), "Digite a senha:");
                                pos = 0; // reset entrada
                                entrada[0] = '\0';
                            }
                        }
                        else if (pos > strlen(senha_trava))
                        {
                            snprintf(msg_display, sizeof(msg_display), "Senha Incorreta!");
                            buzzer_beep(100);
                            sleep_ms(1000);
                            snprintf(msg_display, sizeof(msg_display), "Digite a senha:");
                            pos = 0; // reset entrada
                            entrada[0] = '\0';
                        }
                    }
                }
                if (btn_a_pressionado())
                {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(50);
            }
            break;
        }

        // === Menu principal ===
        case MENU_PRINCIPAL:
        {
            display_menu_principal(menu_idx);

            int8_t dir = joystick_direcao();
            if (dir == 1)
            { // cima
                menu_idx = (menu_idx - 1 + 8) % 8;
            }
            else if (dir == -1)
            { // baixo
                menu_idx = (menu_idx + 1) % 8;
            }

            if (btn_a_pressionado())
            {
                switch (menu_idx)
                {
                case 0:
                    estado = MENU_TEMPERATURA;
                    break;
                case 1:
                    estado = MENU_PRESSAO;
                    break;
                case 2:
                    estado = MENU_LUMINOSIDADE;
                    break;
                case 3:
                    estado = MENU_ACELEROMETRO;
                    break;
                case 4:
                    estado = MENU_HUMIDADE;
                    break;
                case 5:
                    estado = MENU_CONTROLE_TAMPA;
                    break;
                case 6:
                    estado = MENU_CONFIG_TEMPO;
                    break;
                case 7:
                    estado = MENU_TRAVAR_DESTRAVAR;
                    break;
                case 8:
                    estado = MENU_FINALIZAR;
                    break;
                }
            }
            sleep_ms(150);
            break;
        }
        case MENU_HUMIDADE: {
            while (1) {
                float umidade_val = 0.0f;
                float temp_aht = 0.0f;

                // Tenta ler AHT10; se falhar, mostra "--"
                if (!aht10_read_data(sensor_i2c, &umidade_val, &temp_aht)) {
                    display_mensagem_central("Erro leitura\nAHT10");
                } else {
                    char linha1[22];
                    char linha2[22];
                    snprintf(linha1, sizeof(linha1), "Umidade: %.1f%%", umidade_val);
                    snprintf(linha2, sizeof(linha2), "Temp: %.1f C", temp_aht);
                    display_linhas("AHT10", linha1, linha2, "< A para voltar");
                }

                if (btn_a_pressionado()) {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(200);
            }
            break;
        }
        // === Menu Temperatura ===
        case MENU_TEMPERATURA:
        {
            while (1)
            {
                sensors_t dados = bmp280_get_all(0x76);
                char t[22];
                snprintf(t, sizeof(t), "Temp: %.1f C", dados.temperature);

                display_linhas("Temperatura", t, "", "< A para voltar");

                if (btn_a_pressionado())
                {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(200);
            }
            break;
        }

        // === Menu Pressão ===
        case MENU_PRESSAO:
        {
            while (1)
            {
                sensors_t dados = bmp280_get_all(0x76);
                char u[22];
                snprintf(u, sizeof(u), "Press: %.1f hPa", dados.pressure / 100.0);

                display_linhas("Pressao", u, "", "< A para voltar");

                if (btn_a_pressionado())
                {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(200);
            }
            break;
        }

        // === Menu Configuração de Tempo ===
        case MENU_CONFIG_TEMPO:
        {
            // Exibe o cronômetro enquanto estiver neste menu
            while (1) {
                clear_display();
                uint32_t agora = to_ms_since_boot(get_absolute_time());
                uint32_t tempo_decorrido_ms = (tempo_inicio_ms > 0) ? (agora - tempo_inicio_ms) : 0;
                uint32_t horas = tempo_decorrido_ms / (1000 * 60 * 60);
                uint32_t minutos = (tempo_decorrido_ms / (1000 * 60)) % 60;
                uint32_t segundos = (tempo_decorrido_ms / 1000) % 60;
                char tempo_str[32];
                snprintf(tempo_str, sizeof(tempo_str), "%02lu:%02lu:%02lu", horas, minutos, segundos);
                char msg[48];
                snprintf(msg, sizeof(msg), "Cronômetro:\n%s", tempo_str);
                display_mensagem_central(msg);

                // Volta para o menu principal ao apertar B ou A
                if (btn_b_pressionado() || btn_a_pressionado()) {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(200);
            }
            break;
        }

        // === Menu Configuração de Tempo de Alerta ===
        case MENU_CONFIG_TEMPO2:
        {

            while (1)
            {
                // Usar a função de display específica para configuração de alerta
                display_config_tempo_alerta(alerta_tempo_min);

                // Ler joystick para aumentar/diminuir valores
                int8_t dir = joystick_direcao();
                if (dir == 1)
                { // cima
                    if (alerta_tempo_min < 60)
                        alerta_tempo_min++;
                }
                else if (dir == -1)
                { // baixo
                    if (alerta_tempo_min > 1)
                        alerta_tempo_min--;
                }

                // Botão B para confirmar e iniciar a contagem
                if (btn_b_pressionado())
                {

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
                if (btn_a_pressionado())
                {
                    estado = MENU_CONFIG_TEMPO;
                    break;
                }

                sleep_ms(150);
            }
            break;
        }

        // === Menu Luminosidade ===
        case MENU_LUMINOSIDADE:
        {
            while (1)
            {
                uint16_t lux = ler_lux();
                char luxstr[22];
                snprintf(luxstr, sizeof(luxstr), "Lum: %d lux", lux);

                display_linhas("BH1750", luxstr, "", "< A para voltar");

                if (btn_a_pressionado())
                {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(200);
            }
            break;
        }

        // === Menu Acelerômetro ===
        case MENU_ACELEROMETRO:
        {
            while (1)
            {
                int16_t acc[3], gyr[3], temp;
                mpu6050_read_raw(acc, gyr, &temp);

                char acel[22];
                snprintf(acel, sizeof(acel), "X:%d Y:%d Z:%d", acc[0], acc[1], acc[2]);

                display_linhas("Acelerometro", acel, "", "< A para voltar");

                if (btn_a_pressionado())
                {
                    estado = MENU_PRINCIPAL;
                    break;
                }
                sleep_ms(200);
            }
            break;
        }

        // === Menu Configuração de Tempo (Alternativo) ===
        case MENU_CONFIG_TEMPO_ALT:
        {
            uint8_t submenu_idx = 0; // 0: horas, 1: minutos, 2: alerta, 3: salvar
            uint8_t horas = tempo_entrega_ms / (3600 * 1000);
            uint8_t minutos = (tempo_entrega_ms % (3600 * 1000)) / (60 * 1000);
            uint8_t alerta_min = alerta_tempo_min;
            bool configuracao_salva = false;

            while (1)
            {
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

                if (configuracao_salva)
                {
                    ssd1306_draw_string(buffer, 10, 60, "Timer ativado!");
                }

                render_on_display(buffer, &area);

                // Navegação com joystick
                int8_t dir = joystick_direcao();
                if (dir == 1)
                { // cima
                    submenu_idx = (submenu_idx + 3) % 4;
                }
                else if (dir == -1)
                { // baixo
                    submenu_idx = (submenu_idx + 1) % 4;
                }
                else if (dir == 2)
                { // direita
                    // Aumentar valor
                    switch (submenu_idx)
                    {
                    case 0:                        // horas
                        horas = (horas + 1) % 100; // máximo 99 horas
                        break;
                    case 1: // minutos
                        minutos = (minutos + 1) % 60;
                        break;
                    case 2:                                 // alerta
                        alerta_min = (alerta_min + 1) % 61; // máximo 60 minutos
                        break;
                    }
                }
                else if (dir == -2)
                { // esquerda
                    // Diminuir valor
                    switch (submenu_idx)
                    {
                    case 0:                         // horas
                        horas = (horas + 99) % 100; // máximo 99 horas
                        break;
                    case 1: // minutos
                        minutos = (minutos + 59) % 60;
                        break;
                    case 2:                                  // alerta
                        alerta_min = (alerta_min + 60) % 61; // máximo 60 minutos
                        break;
                    }
                }

                // Botão A para voltar
                if (btn_a_pressionado())
                {
                    estado = MENU_PRINCIPAL;
                    break;
                }

                // Botão B para confirmar/salvar
                if (btn_b_pressionado())
                {
                    if (submenu_idx == 3)
                    { // Opção Salvar
                        // Calcular tempo total em ms
                        tempo_entrega_ms = (horas * 3600 + minutos * 60) * 1000;
                        alerta_tempo_min = alerta_min;

                        if (tempo_entrega_ms > 0)
                        {
                            // Iniciar contagem
                            tempo_inicio_ms = to_ms_since_boot(get_absolute_time());
                            alarme_tempo_ativo = true;
                            configuracao_salva = true;
                            led_verde(true);  // LED verde indica timer ativo
                            buzzer_beep(200); // Beep curto para confirmar
                        }
                        else
                        {
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

        // === Finalizar ===
        case MENU_FINALIZAR:
        {
            display_mensagem_central("Finalizando...");
            sleep_ms(2000);
            // Resetar cronômetro ao finalizar transporte
            tempo_inicio_ms = 0;
            tempo_entrega_ms = 0;
            tempo_restante_ms = 0;
            display_mensagem_central("Ate logo!");
            sleep_ms(2000);
            estado = TELA_BEM_VINDO;
            break;
        }
        // Fim do switch(estado)
        } // <--- fecha o switch(estado)
    } // <--- fecha o while(1)
    return 0;
}