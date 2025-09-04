#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdbool.h>
#include <stdint.h>

// Estrutura para armazenar os dados dos sensores
typedef struct {
    float temperatura;
    float pressao;
    uint16_t luminosidade;
    int16_t aceleracao_x;
    int16_t aceleracao_y;
    int16_t aceleracao_z;
    bool caixa_aberta;
    // Novos campos para o contador de tempo e alertas
    uint32_t tempo_entrega_ms;      // Tempo configurado para entrega em milissegundos
    uint32_t tempo_restante_ms;     // Tempo restante para entrega em milissegundos
    uint32_t alerta_tempo_min;      // Minutos antes do fim para acionar alerta
    bool alerta_temp_ativo;         // Indica se o alerta de temperatura está ativo
    bool alerta_acel_ativo;         // Indica se o alerta de aceleração está ativo
    bool alarme_tempo_ativo;        // Indica se o alarme de tempo está ativo
    bool alerta_tempo_ativo;        // Indica se o alerta de tempo está ativo
    char data_hora[24];             // String com data e hora atual (formato: YYYY-MM-DD HH:MM:SS)
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