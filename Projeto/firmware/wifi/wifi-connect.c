#include "wifi-connect.h"

// Variável global para controlar o estado da conexão
static bool wifi_initialized = false;

// Inicialização do wifi e conectando na rede
bool init_wifi(void)
{
    if (wifi_initialized) {
        return true; // Já inicializado
    }
    
    // Inicialização do Wi-Fi
    if (cyw43_arch_init())
    {
        printf("Erro ao inicializar Wi-Fi\n");
        return false;
    }

    // Ativação do modo STA (cliente)
    cyw43_arch_enable_sta_mode();

    // Conectando ao Wi-Fi com timeout definido
    int attempts = 0;
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS))
    {
        printf("Erro ao conectar ao Wi-Fi: tentando novamente (%d)...\n", ++attempts);
        
        // Após várias tentativas, desistir
        if (attempts >= 5) {
            printf("Falha ao conectar ao Wi-Fi após %d tentativas\n", attempts);
            cyw43_arch_deinit();
            return false;
        }
        
        sleep_ms(WIFI_RETRY_DELAY_MS);
    }

    printf("Wi-Fi conectado com sucesso!\n");
    wifi_initialized = true;
    return true;
}

// Verifica se o WiFi está conectado
bool is_wifi_connected(void) {
    if (!wifi_initialized) {
        return false;
    }
    
    // Verifica o status da conexão
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

// Reconecta ao WiFi se necessário
bool reconnect_wifi(void) {
    if (!wifi_initialized) {
        return init_wifi();
    }
    
    if (is_wifi_connected()) {
        return true; // Já está conectado
    }
    
    printf("Reconectando ao Wi-Fi...\n");
    
    // Tenta reconectar
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS)) {
        printf("Erro ao reconectar ao Wi-Fi\n");
        return false;
    }
    
    printf("Wi-Fi reconectado com sucesso!\n");
    return true;
}

// Finaliza a conexão WiFi
void deinit_wifi(void) {
    if (!wifi_initialized) {
        return;
    }
    
    cyw43_arch_deinit();
    wifi_initialized = false;
    printf("Wi-Fi finalizado\n");
}
