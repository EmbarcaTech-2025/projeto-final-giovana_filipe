#ifndef WIFI_DASHBOARD_H
#define WIFI_DASHBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Configurações WiFi
#define WIFI_SSID "BioSmartCooler_AP"
#define WIFI_PASSWORD "biosmart123"
#define WIFI_CHANNEL 6
#define WIFI_MAX_CONNECTIONS 4

// Configurações do servidor web
#define WEB_SERVER_PORT 80
#define WEB_SERVER_MAX_CLIENTS 4
#define WEB_SOCKET_PORT 81

// Configurações de segurança
#define REMOTE_PASSWORD "1234"
#define MAX_LOGIN_ATTEMPTS 3
#define SESSION_TIMEOUT 300000 // 5 minutos

// Estados do sistema
typedef enum {
    SYSTEM_IDLE,
    SYSTEM_RECORDING,
    SYSTEM_ALARM,
    SYSTEM_ERROR
} system_state_t;

// Estrutura para dados em tempo real
typedef struct {
    time_t timestamp;
    float temperatura;
    float pressao;
    uint16_t luminosidade;
    int16_t aceleracao_x;
    int16_t aceleracao_y;
    int16_t aceleracao_z;
    uint8_t umidade;
    bool caixa_aberta;
    system_state_t estado_sistema;
    bool logging_ativo;
    uint32_t registros_totais;
} realtime_data_t;

// Estrutura para configuração do dashboard
typedef struct {
    char ssid[32];
    char password[64];
    uint16_t port;
    bool ap_mode;
    bool recording_enabled;
    char remote_password[16];
    uint32_t update_interval_ms;
} dashboard_config_t;

// Funções de inicialização e controle WiFi
bool wifi_dashboard_init(void);
void wifi_dashboard_deinit(void);
bool wifi_dashboard_is_connected(void);
bool wifi_dashboard_start_ap(void);
bool wifi_dashboard_connect_station(const char* ssid, const char* password);

// Funções do servidor web
bool wifi_dashboard_start_server(void);
void wifi_dashboard_stop_server(void);
bool wifi_dashboard_is_server_running(void);

// Funções de dados em tempo real
bool wifi_dashboard_send_realtime_data(const realtime_data_t* data);
bool wifi_dashboard_broadcast_system_state(system_state_t state);
bool wifi_dashboard_send_alarm(const char* alarm_type, const char* message);

// Funções de controle remoto
bool wifi_dashboard_verify_password(const char* password);
bool wifi_dashboard_unlock_box(const char* password);
bool wifi_dashboard_start_recording(void);
bool wifi_dashboard_stop_recording(void);
bool wifi_dashboard_download_csv(void);

// Funções de configuração
void wifi_dashboard_set_config(const dashboard_config_t* config);
dashboard_config_t wifi_dashboard_get_config(void);
void wifi_dashboard_set_update_interval(uint32_t interval_ms);
uint32_t wifi_dashboard_get_update_interval(void);

// Funções de status
bool wifi_dashboard_get_box_status(void);
system_state_t wifi_dashboard_get_system_state(void);
uint32_t wifi_dashboard_get_connected_clients(void);
uint32_t wifi_dashboard_get_total_records(void);

// Funções de diagnóstico
bool wifi_dashboard_test_connection(void);
bool wifi_dashboard_test_server(void);
uint32_t wifi_dashboard_get_signal_strength(void);
bool wifi_dashboard_get_ip_address(char* ip_buffer, size_t buffer_size);

// Callbacks para eventos
typedef void (*wifi_client_connected_callback_t)(void);
typedef void (*wifi_client_disconnected_callback_t)(void);
typedef void (*wifi_recording_start_callback_t)(void);
typedef void (*wifi_recording_stop_callback_t)(void);
typedef void (*wifi_unlock_request_callback_t)(const char* password);

void wifi_dashboard_set_client_connected_callback(wifi_client_connected_callback_t callback);
void wifi_dashboard_set_client_disconnected_callback(wifi_client_disconnected_callback_t callback);
void wifi_dashboard_set_recording_start_callback(wifi_recording_start_callback_t callback);
void wifi_dashboard_set_recording_stop_callback(wifi_recording_stop_callback_t callback);
void wifi_dashboard_set_unlock_request_callback(wifi_unlock_request_callback_t callback);

// Funções de atualização de status
void wifi_dashboard_update_box_status(bool is_open);
void wifi_dashboard_update_total_records(uint32_t total);

#endif // WIFI_DASHBOARD_H 