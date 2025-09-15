#include "send-data.h"
#include <string.h>

// Variável global para armazenar a única conexão TCP persistente
static struct tcp_pcb *pcb_global = NULL;
static bool is_connecting = false;

// Variável para armazenar comando recebido
static char received_command[256] = {0};
static bool has_command = false;

// Callback para quando a conexão TCP é estabelecida
static err_t connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Erro ao conectar ao servidor: %d\n", err);
        tcp_abort(tpcb);
        pcb_global = NULL;
        is_connecting = false;
        return ERR_ABRT;
    }
    printf("Conexão com o servidor estabelecida com sucesso!\n");
    is_connecting = false;
    return ERR_OK;
}

// Callback para quando os dados são enviados com sucesso
static err_t sent_callback(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    printf("Dados enviados com sucesso! Bytes: %u\n", len);
    // A conexão não é fechada aqui para ser reutilizada
    return ERR_OK;
}

// Callback para quando a conexão TCP é fechada remotamente
static err_t recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p) {
        // Processar os dados recebidos
        char *data = (char *)p->payload;
        printf("Dados recebidos: %s\n", data);
        
        // Verificar se há comando
        if (strstr(data, "\"comando\":{")) {
            // Extrair o comando (simplificado)
            char *cmd_start = strstr(data, "\"comando\":\"");
            if (cmd_start) {
                cmd_start += 11; // Avançar após "comando":"
                char *cmd_end = strchr(cmd_start, '"');
                if (cmd_end) {
                    size_t len = cmd_end - cmd_start;
                    if (len < sizeof(received_command)) {
                        strncpy(received_command, cmd_start, len);
                        received_command[len] = '\0';
                        has_command = true;
                        printf("Comando recebido: %s\n", received_command);
                    }
                }
            }
        }
        
        pbuf_free(p);
    } else if (err == ERR_OK) {
        // Conexão fechada remotamente
        printf("Conexão com o servidor fechada.\n");
        tcp_close(pcb);
        pcb_global = NULL;
    }
    return ERR_OK;
}

// Inicia uma nova conexão persistente se não houver uma ativa
bool open_persistent_connection(void) {
    if (pcb_global != NULL || is_connecting) {
        return true; // Conexão já existe ou está em progresso
    }

    pcb_global = tcp_new();
    if (!pcb_global) {
        printf("Erro ao criar PCB\n");
        return false;
    }
    
    is_connecting = true;
    
    ip_addr_t server_ip;
    server_ip.addr = ipaddr_addr(SERVER_IP);
    
    // Conecta ao servidor e define o callback de sucesso
    err_t err = tcp_connect(pcb_global, &server_ip, SERVER_PORT, connected_callback);
    if (err != ERR_OK) {
        printf("Erro ao iniciar a conexão: %d\n", err);
        tcp_abort(pcb_global);
        pcb_global = NULL;
        is_connecting = false;
        return false;
    }
    
    // Define os callbacks para gerenciar o estado da conexão
    tcp_sent(pcb_global, sent_callback);
    tcp_recv(pcb_global, recv_callback);

    return true;
}

// Envia dados para o servidor usando a conexão persistente
err_t send_data_to_server_persistent(const char *path, char *request_body, const char *type_method)
{
    // Verifica se a conexão está ativa antes de tentar enviar
    if (pcb_global == NULL || pcb_global->state != ESTABLISHED) {
        printf("Conexão não estabelecida. Tentando reconectar...\n");
        if (!open_persistent_connection()) {
            printf("Falha ao abrir a conexão persistente.\n");
            return ERR_CONN;
        }
        // Aguarda a conexão ser estabelecida (simplificado)
        // Em um ambiente real, um mecanismo de estado mais robusto seria necessário
        for (int i = 0; i < 20; i++) {
            if (pcb_global != NULL && pcb_global->state == ESTABLISHED) {
                break;
            }
            sleep_ms(100);
        }
        if (pcb_global == NULL || pcb_global->state != ESTABLISHED) {
            printf("A conexão não pôde ser restabelecida em tempo hábil.\n");
            return ERR_TIMEOUT;
        }
    }

    // Montando requisição HTTP
    char request[768];
    snprintf(request, sizeof(request),
             "%s %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             type_method, path, SERVER_IP, strlen(request_body), request_body);

    // Empacotando e enviando a requisição
    err_t err = tcp_write(pcb_global, request, strlen(request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("Erro ao empacotar os dados: %d\n", err);
        return err;
    }
    
    err = tcp_output(pcb_global);
    if (err != ERR_OK) {
        printf("Erro ao enviar os dados (tcp_output): %d\n", err);
        return err;
    }
    
    return ERR_OK;
}

// Funções existentes que usam o novo método de envio
void send_data_to_server(const char *path, char *request_body, const char *type_method)
{
    // A função original foi modificada para usar o novo método
    send_data_to_server_persistent(path, request_body, type_method);
}

void create_request(int data)
{
    const char *type_method = "POST";
    const char *path = SERVER_PATH;
    char json_request[256];

    snprintf(json_request, sizeof(json_request),
             "{ \"dado\" : %d }",
             data);

    send_data_to_server_persistent(path, json_request, type_method);
}

bool send_sensor_data(float temperatura, float pressao, uint16_t luminosidade, 
                      int16_t accel_x, int16_t accel_y, int16_t accel_z, 
                      bool caixa_aberta) {
    const char *type_method = "POST";
    const char *path = "/sensores";
    char json_request[512];

    snprintf(json_request, sizeof(json_request),
             "{"
             "\"temperatura\": %.2f,"
             "\"pressao\": %.2f,"
             "\"luminosidade\": %u,"
             "\"aceleracao_x\": %d,"
             "\"aceleracao_y\": %d,"
             "\"aceleracao_z\": %d,"
             "\"caixa_aberta\": %s"
             "}",
             temperatura,
             pressao,
             luminosidade,
             accel_x,
             accel_y,
             accel_z,
             caixa_aberta ? "true" : "false");

    err_t result = send_data_to_server_persistent(path, json_request, type_method);
    return result == ERR_OK;
}

// Função para obter comando pendente
bool get_pending_command(char *command, size_t max_len) {
    if (has_command) {
        strncpy(command, received_command, max_len - 1);
        command[max_len - 1] = '\0';
        has_command = false;
        return true;
    }
    return false;
}