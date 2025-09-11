#include "dashboard.h"
#include "firmware/send-data-to-server/send-data.h"
#include "firmware/wifi/wifi-connect.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Variáveis globais
static bool dashboard_initialized = false;
static sensor_data_t last_sensor_data = {0};

bool dashboard_init(void) {
    if (dashboard_initialized) return true;
    if (!init_wifi()) {
        printf("Falha ao inicializar WiFi para o dashboard\n");
        return false;
    }
    dashboard_initialized = true;
    printf("Dashboard inicializado com sucesso\n");
    return true;
}

bool dashboard_update_sensor_data(const sensor_data_t *data) {
    if (!dashboard_initialized || !data) {
        return false;
    }

    memcpy(&last_sensor_data, data, sizeof(sensor_data_t));

    const char *status_caixa = data->caixa_aberta ? "ABERTA" : "FECHADA";

    char json_data[1400];
    snprintf(json_data, sizeof(json_data),
             "{"
             "\"temperatura\": %.2f,"
             "\"pressao\": %.2f,"
             "\"umidade\": %2f,"
             "\"luminosidade\": %u,"
             "\"aceleracao_x\": %d,"
             "\"aceleracao_y\": %d,"
             "\"aceleracao_z\": %d,"
             "\"aceleracao_total\": %.2f,"
             "\"caixa_aberta\": %s,"
             "\"status_caixa\": \"%s\"," 
             "\"tempo_entrega_ms\": %u,"
             "\"tempo_restante_ms\": %u,"
             "\"alerta_tempo_min\": %u,"
             "\"alerta_temp_ativo\": %s,"
             "\"alerta_acel_ativo\": %s,"
             "\"alarme_tempo_ativo\": %s,"
             "\"alerta_tempo_ativo\": %s,"
             "\"data_hora\": \"%s\"," 
             "\"led_status\": \"%s\"," 
             "\"registros_totais\": %d,"
             "\"tempo_decorrido_ms\": %u"
             "}",
             data->temperatura,
             data->pressao,
             data->umidade,
             data->luminosidade,
             data->aceleracao_x,
             data->aceleracao_y,
             data->aceleracao_z,
             data->aceleracao_total,
             data->caixa_aberta ? "true" : "false",
             status_caixa,
             data->tempo_entrega_ms,
             data->tempo_restante_ms,
             data->alerta_tempo_min,
             data->alerta_temp_ativo ? "true" : "false",
             data->alerta_acel_ativo ? "true" : "false",
             data->alarme_tempo_ativo ? "true" : "false",
             data->alerta_tempo_ativo ? "true" : "false",
             data->data_hora,
             data->led_status,
             data->total_registros,
             data->tempo_decorrido_ms);

    printf("[DASHBOARD] Enviando dados: %s\n", json_data);
    send_data_to_server("/sensores", json_data, "POST");
    return true;
}

bool dashboard_check_commands(void) {
    return false;
}

bool dashboard_process_command(const char *command) {
    if (!dashboard_initialized || !command) return false;
    printf("Processando comando: %s\n", command);
    return true;
}

void dashboard_deinit(void) {
    if (!dashboard_initialized) return;
    dashboard_initialized = false;
    printf("Dashboard finalizado\n");
}
