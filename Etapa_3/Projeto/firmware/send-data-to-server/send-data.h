#include "lwip/tcp.h"     // biblioteca que gerencia conexões TCP/IP
#include "lwip/ip_addr.h" // Lida com endereços IP.
#include "pico/cyw43_arch.h"
#include <stdbool.h>

// Configurações do servidor
#define SERVER_IP "192.168.184.209"  // IP do servidor (Raspberry Pi Pico W)
#define SERVER_PORT 3000              // Porta do servidor
#define SERVER_PATH "/sensores"      // Rota padrão para envio dos dados

// Envia dados para o servidor
void send_data_to_server(const char *path, char *request_body, const char *type_method);

// Cria e envia uma requisição simples com um valor inteiro
void create_request(int data);

// Cria e envia uma requisição com dados de sensores
bool send_sensor_data(float temperatura, float pressao, uint16_t luminosidade, 
                      int16_t accel_x, int16_t accel_y, int16_t accel_z, 
                      bool caixa_aberta);