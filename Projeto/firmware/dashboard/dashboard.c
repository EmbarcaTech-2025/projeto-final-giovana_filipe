#include "dashboard.h"
#include "firmware/send-data-to-server/send-data.h"
#include "firmware/wifi/wifi-connect.h"
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
    
    // CORREÇÃO: Chamar a função send_sensor_data em vez de construir o JSON manualmente
    printf("[DASHBOARD] Enviando dados para o servidor: temperatura=%.2f, pressao=%.2f, luminosidade=%u, accel_x=%d, accel_y=%d, accel_z=%d, caixa_aberta=%d\n",
        data->temperatura, data->pressao, data->luminosidade, data->aceleracao_x, data->aceleracao_y, data->aceleracao_z, data->caixa_aberta);

    send_sensor_data(data->temperatura, data->pressao, data->luminosidade,
                     data->aceleracao_x, data->aceleracao_y, data->aceleracao_z,
                     data->caixa_aberta);
    
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
