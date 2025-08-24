
#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Configurações WiFi
#define WIFI_SSID "FilipeSousa"
#define WIFI_PASSWORD "devlove28"
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define WIFI_RETRY_DELAY_MS 3000

// Inicializa a conexão WiFi
bool init_wifi(void);

// Verifica se o WiFi está conectado
bool is_wifi_connected(void);

// Reconecta ao WiFi se necessário
bool reconnect_wifi(void);

// Finaliza a conexão WiFi
void deinit_wifi(void);

#endif // WIFI_CONNECT_H