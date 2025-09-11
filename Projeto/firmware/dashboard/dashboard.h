#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdbool.h>
#include <stdint.h>

// Estrutura para armazenar os dados dos sensores
typedef struct {
    float temperatura;
    float pressao;
    float umidade;              // << NOVO
    uint16_t luminosidade;
    int16_t aceleracao_x;
    int16_t aceleracao_y;
    int16_t aceleracao_z;
    float aceleracao_total;
    bool caixa_aberta;
    uint32_t tempo_entrega_ms;
    uint32_t tempo_restante_ms;
    uint32_t alerta_tempo_min;
    bool alerta_temp_ativo;
    bool alerta_acel_ativo;
    bool alarme_tempo_ativo;
    bool alerta_tempo_ativo;
    char data_hora[24];
    char led_status[16];
    int total_registros;
    uint32_t tempo_decorrido_ms; // NOVO: tempo cronometrado em ms
} sensor_data_t;

// Inicializa o módulo de dashboard
bool dashboard_init(void);

// Atualiza os dados dos sensores no dashboard
bool dashboard_update_sensor_data(const sensor_data_t *data);

// Verifica se há comandos pendentes do dashboard
bool dashboard_check_commands(void);

// Processa comandos recebidos do dashboard
bool dashboard_process_command(const char *command);

// Finaliza o módulo de dashboard
void dashboard_deinit(void);

#endif // DASHBOARD_H
