#include "dashboard.h"
#include "../send-data-to-server/send-data.h"
#include "../wifi-connection/wifi-connect.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Variáveis globais
static bool dashboard_initialized = false;
static sensor_data_t last_sensor_data = {0};

// Inicializa o módulo de dashboard
bool dashboard_init(void) {
    if (dashboard_initialized) {
        return true; // Já inicializado
    }
    
    // Inicializa a conexão WiFi
    if (!init_wifi()) {
        printf("Falha ao inicializar WiFi para o dashboard\n");
        return false;
    }
    
    dashboard_initialized = true;
    printf("Dashboard inicializado com sucesso\n");
    return true;
}

// Atualiza os dados dos sensores no dashboard
bool dashboard_update_sensor_data(const sensor_data_t *data) {
    if (!dashboard_initialized || !data) {
        return false;
    }
    
    // Copia os dados para a variável global
    memcpy(&last_sensor_data, data, sizeof(sensor_data_t));
    
    // Prepara o JSON com os dados dos sensores
    char json_data[768]; // Aumentado para acomodar os novos campos
    printf("[DASHBOARD] Enviando dados para o servidor: temperatura=%.2f, pressao=%.2f, luminosidade=%u, accel_x=%d, accel_y=%d, accel_z=%d, caixa_aberta=%d, tempo_entrega_ms=%u, tempo_restante_ms=%u, alerta_tempo_min=%u, alerta_temp_ativo=%d, alerta_acel_ativo=%d, alarme_tempo_ativo=%d, alerta_tempo_ativo=%d, data_hora=%s\n",
        data->temperatura, data->pressao, data->luminosidade, data->aceleracao_x, data->aceleracao_y, data->aceleracao_z, data->caixa_aberta, data->tempo_entrega_ms, data->tempo_restante_ms, data->alerta_tempo_min, data->alerta_temp_ativo, data->alerta_acel_ativo, data->alarme_tempo_ativo, data->alerta_tempo_ativo, data->data_hora);
    
    // Também imprime o JSON gerado
    snprintf(json_data, sizeof(json_data),
             "{"
             "\"temperatura\": %.2f,"
             "\"pressao\": %.2f,"
             "\"luminosidade\": %u,"
             "\"aceleracao_x\": %d,"
             "\"aceleracao_y\": %d,"
             "\"aceleracao_z\": %d,"
             "\"caixa_aberta\": %s,"
             "\"tempo_entrega_ms\": %u,"
             "\"tempo_restante_ms\": %u,"
             "\"alerta_tempo_min\": %u,"
             "\"alerta_temp_ativo\": %s,"
             "\"alerta_acel_ativo\": %s,"
             "\"alarme_tempo_ativo\": %s,"
             "\"alerta_tempo_ativo\": %s,"
             "\"data_hora\": \"%s\""
             "}",
             data->temperatura,
             data->pressao,
             data->luminosidade,
             data->aceleracao_x,
             data->aceleracao_y,
             data->aceleracao_z,
             data->caixa_aberta ? "true" : "false",
             data->tempo_entrega_ms,
             data->tempo_restante_ms,
             data->alerta_tempo_min,
             data->alerta_temp_ativo ? "true" : "false",
             data->alerta_acel_ativo ? "true" : "false",
             data->alarme_tempo_ativo ? "true" : "false",
             data->alerta_tempo_ativo ? "true" : "false",
             data->data_hora);
    
    // Envia os dados para o servidor
    send_data_to_server("/sensores", json_data, "POST");
    
    return true;
}

// Verifica se há comandos pendentes do dashboard
bool dashboard_check_commands(void) {
    // Esta função seria implementada com uma chamada ao servidor
    // para verificar se há comandos pendentes
    // Por enquanto, retorna false (sem comandos)
    return false;
}

// Processa comandos recebidos do dashboard
bool dashboard_process_command(const char *command) {
    if (!dashboard_initialized || !command) {
        return false;
    }
    
    // Implementação básica para processar comandos
    // Aqui seria implementada a lógica para interpretar e executar comandos
    printf("Processando comando: %s\n", command);
    
    return true;
}

// Finaliza o módulo de dashboard
void dashboard_deinit(void) {
    if (!dashboard_initialized) {
        return;
    }
    
    // Código para finalizar o dashboard
    dashboard_initialized = false;
    printf("Dashboard finalizado\n");
}