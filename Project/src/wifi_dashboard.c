#include "inc/wifi_dashboard.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Variáveis globais
static bool wifi_initialized = false;
static bool server_running = false;
static bool recording_enabled = false;
static uint32_t connected_clients = 0;
static uint32_t total_records = 0;
static system_state_t current_state = SYSTEM_IDLE;
static bool box_status = false; // false = fechada, true = aberta

// Callbacks
static wifi_client_connected_callback_t client_connected_cb = NULL;
static wifi_client_disconnected_callback_t client_disconnected_cb = NULL;
static wifi_recording_start_callback_t recording_start_cb = NULL;
static wifi_recording_stop_callback_t recording_stop_cb = NULL;
static wifi_unlock_request_callback_t unlock_request_cb = NULL;

// Configuração padrão
static dashboard_config_t config = {
    .ssid = "BioSmartCooler_AP",
    .password = "biosmart123",
    .port = 80,
    .ap_mode = true,
    .recording_enabled = false,
    .remote_password = "1234",
    .update_interval_ms = 1000
};

// Buffer para dados em tempo real
static realtime_data_t current_data = {0};

// Inicializa o sistema WiFi
bool wifi_dashboard_init(void) {
    if (wifi_initialized) return true;
    
    // Inicializa o chip WiFi
    if (cyw43_arch_init()) {
        printf("Erro: Falha ao inicializar WiFi\n");
        return false;
    }
    
    // Habilita o WiFi
    cyw43_arch_enable_sta_mode();
    
    // Configura o IP estático para modo AP
    if (config.ap_mode) {
        ip4_addr_t ipaddr, netmask, gw;
        IP4_ADDR(&ipaddr, 192, 168, 4, 1);
        IP4_ADDR(&netmask, 255, 255, 255, 0);
        IP4_ADDR(&gw, 192, 168, 4, 1);
        
        netif_set_addr(netif_default, &ipaddr, &netmask, &gw);
    }
    
    wifi_initialized = true;
    printf("WiFi inicializado com sucesso\n");
    return true;
}

void wifi_dashboard_deinit(void) {
    if (!wifi_initialized) return;
    
    wifi_dashboard_stop_server();
    cyw43_arch_deinit();
    wifi_initialized = false;
}

bool wifi_dashboard_is_connected(void) {
    return wifi_initialized && cyw43_arch_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

// Inicia o modo Access Point
bool wifi_dashboard_start_ap(void) {
    if (!wifi_initialized) return false;
    
    cyw43_arch_enable_ap_mode(config.ssid, config.password, WIFI_CHANNEL);
    
    // Aguarda a conexão
    while (cyw43_arch_wifi_link_status(&cyw43_state, CYW43_ITF_AP) != CYW43_LINK_UP) {
        sleep_ms(100);
    }
    
    printf("Access Point iniciado: %s\n", config.ssid);
    return true;
}

// Conecta a uma rede WiFi
bool wifi_dashboard_connect_station(const char* ssid, const char* password) {
    if (!wifi_initialized) return false;
    
    cyw43_arch_enable_sta_mode();
    
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Erro: Falha ao conectar ao WiFi\n");
        return false;
    }
    
    printf("Conectado ao WiFi: %s\n", ssid);
    return true;
}

// Inicia o servidor web
bool wifi_dashboard_start_server(void) {
    if (server_running) return true;
    
    // Inicializa o sistema de arquivos HTTP
    httpd_init();
    
    // Registra os callbacks para páginas específicas
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
    http_set_ssi_handler(ssi_handler, LWIP_ARRAYSIZE(ssi_tags), LWIP_ARRAYSIZE(ssi_tags));
    
    server_running = true;
    printf("Servidor web iniciado na porta %d\n", config.port);
    return true;
}

void wifi_dashboard_stop_server(void) {
    if (!server_running) return;
    
    httpd_deinit();
    server_running = false;
    printf("Servidor web parado\n");
}

bool wifi_dashboard_is_server_running(void) {
    return server_running;
}

// Envia dados em tempo real via WebSocket
bool wifi_dashboard_send_realtime_data(const realtime_data_t* data) {
    if (!data) return false;
    
    // Atualiza dados atuais
    memcpy(&current_data, data, sizeof(realtime_data_t));
    
    // Envia via WebSocket para todos os clientes conectados
    // Esta é uma implementação simplificada - em produção seria mais robusta
    char json_buffer[512];
    snprintf(json_buffer, sizeof(json_buffer),
        "{\"timestamp\":%ld,\"temperatura\":%.2f,\"pressao\":%.2f,"
        "\"luminosidade\":%d,\"aceleracao_x\":%d,\"aceleracao_y\":%d,"
        "\"aceleracao_z\":%d,\"umidade\":%d,\"caixa_aberta\":%s,"
        "\"estado_sistema\":%d,\"logging_ativo\":%s,\"registros_totais\":%lu}",
        data->timestamp, data->temperatura, data->pressao,
        data->luminosidade, data->aceleracao_x, data->aceleracao_y,
        data->aceleracao_z, data->umidade, data->caixa_aberta ? "true" : "false",
        data->estado_sistema, data->logging_ativo ? "true" : "false",
        data->registros_totais);
    
    // Envia via WebSocket (implementação simplificada)
    printf("Dados enviados: %s\n", json_buffer);
    return true;
}

bool wifi_dashboard_broadcast_system_state(system_state_t state) {
    current_state = state;
    
    char state_message[128];
    snprintf(state_message, sizeof(state_message),
        "{\"type\":\"state_change\",\"state\":%d,\"timestamp\":%ld}",
        state, time(NULL));
    
    printf("Estado do sistema: %s\n", state_message);
    return true;
}

bool wifi_dashboard_send_alarm(const char* alarm_type, const char* message) {
    char alarm_message[256];
    snprintf(alarm_message, sizeof(alarm_message),
        "{\"type\":\"alarm\",\"alarm_type\":\"%s\",\"message\":\"%s\",\"timestamp\":%ld}",
        alarm_type, message, time(NULL));
    
    printf("Alarme enviado: %s\n", alarm_message);
    return true;
}

// Verifica senha remota
bool wifi_dashboard_verify_password(const char* password) {
    return strcmp(password, config.remote_password) == 0;
}

// Destrava a caixa remotamente
bool wifi_dashboard_unlock_box(const char* password) {
    if (!wifi_dashboard_verify_password(password)) {
        return false;
    }
    
    if (unlock_request_cb) {
        unlock_request_cb(password);
    }
    
    printf("Caixa destravada remotamente\n");
    return true;
}

// Inicia gravação
bool wifi_dashboard_start_recording(void) {
    if (recording_enabled) return false;
    
    recording_enabled = true;
    
    if (recording_start_cb) {
        recording_start_cb();
    }
    
    printf("Gravação iniciada remotamente\n");
    return true;
}

// Para gravação
bool wifi_dashboard_stop_recording(void) {
    if (!recording_enabled) return false;
    
    recording_enabled = false;
    
    if (recording_stop_cb) {
        recording_stop_cb();
    }
    
    printf("Gravação parada remotamente\n");
    return true;
}

// Download do CSV
bool wifi_dashboard_download_csv(void) {
    // Esta função seria implementada para servir o arquivo CSV
    // via HTTP para download
    printf("Download CSV solicitado\n");
    return true;
}

// Configurações
void wifi_dashboard_set_config(const dashboard_config_t* new_config) {
    if (new_config) {
        memcpy(&config, new_config, sizeof(dashboard_config_t));
    }
}

dashboard_config_t wifi_dashboard_get_config(void) {
    return config;
}

void wifi_dashboard_set_update_interval(uint32_t interval_ms) {
    config.update_interval_ms = interval_ms;
}

uint32_t wifi_dashboard_get_update_interval(void) {
    return config.update_interval_ms;
}

// Status
bool wifi_dashboard_get_box_status(void) {
    return box_status;
}

system_state_t wifi_dashboard_get_system_state(void) {
    return current_state;
}

uint32_t wifi_dashboard_get_connected_clients(void) {
    return connected_clients;
}

uint32_t wifi_dashboard_get_total_records(void) {
    return total_records;
}

// Diagnóstico
bool wifi_dashboard_test_connection(void) {
    return wifi_initialized && wifi_dashboard_is_connected();
}

bool wifi_dashboard_test_server(void) {
    return server_running;
}

uint32_t wifi_dashboard_get_signal_strength(void) {
    if (!wifi_initialized) return 0;
    
    // Retorna força do sinal em dBm (simplificado)
    return cyw43_arch_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP ? -50 : -100;
}

bool wifi_dashboard_get_ip_address(char* ip_buffer, size_t buffer_size) {
    if (!ip_buffer || buffer_size < 16) return false;
    
    if (config.ap_mode) {
        snprintf(ip_buffer, buffer_size, "192.168.4.1");
    } else {
        // Obtém IP da interface STA
        snprintf(ip_buffer, buffer_size, "192.168.1.100"); // Simplificado
    }
    
    return true;
}

// Callbacks
void wifi_dashboard_set_client_connected_callback(wifi_client_connected_callback_t callback) {
    client_connected_cb = callback;
}

void wifi_dashboard_set_client_disconnected_callback(wifi_client_disconnected_callback_t callback) {
    client_disconnected_cb = callback;
}

void wifi_dashboard_set_recording_start_callback(wifi_recording_start_callback_t callback) {
    recording_start_cb = callback;
}

void wifi_dashboard_set_recording_stop_callback(wifi_recording_stop_callback_t callback) {
    recording_stop_cb = callback;
}

void wifi_dashboard_set_unlock_request_callback(wifi_unlock_request_callback_t callback) {
    unlock_request_cb = callback;
}

// Callbacks para eventos de clientes
void wifi_dashboard_on_client_connected(void) {
    connected_clients++;
    printf("Cliente conectado. Total: %lu\n", connected_clients);
    
    if (client_connected_cb) {
        client_connected_cb();
    }
}

void wifi_dashboard_on_client_disconnected(void) {
    if (connected_clients > 0) {
        connected_clients--;
    }
    printf("Cliente desconectado. Total: %lu\n", connected_clients);
    
    if (client_disconnected_cb) {
        client_disconnected_cb();
    }
}

// Atualiza status da caixa
void wifi_dashboard_update_box_status(bool is_open) {
    box_status = is_open;
    printf("Status da caixa: %s\n", is_open ? "ABERTA" : "FECHADA");
}

// Atualiza total de registros
void wifi_dashboard_update_total_records(uint32_t total) {
    total_records = total;
} 