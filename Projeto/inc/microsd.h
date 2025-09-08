#ifndef MICROSD_H
#define MICROSD_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Configurações SPI para microSD
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// Configurações de arquivo
#define MAX_FILENAME_LEN 32
#define MAX_LINE_LEN 256
#define CSV_HEADER "Timestamp,Temperatura(C),Pressao(hPa),Luminosidade(lux),Aceleracao_X,Aceleracao_Y,Aceleracao_Z,Umidade(%)\n"

// Estrutura para dados dos sensores
typedef struct {
    time_t timestamp;
    float temperatura;
    float pressao;
    uint16_t luminosidade;
    int16_t aceleracao_x;
    int16_t aceleracao_y;
    int16_t aceleracao_z;
    uint8_t umidade;
} sensor_data_t;

// Estrutura para configuração de logging
typedef struct {
    char filename[MAX_FILENAME_LEN];
    bool auto_create;
    uint32_t sample_interval_ms;
    bool header_written;
} logging_config_t;

// Funções de inicialização e controle
bool microsd_init(void);
void microsd_deinit(void);
bool microsd_is_mounted(void);

// Funções de arquivo
bool microsd_create_file(const char* filename);
bool microsd_write_header(const char* filename);
bool microsd_append_data(const char* filename, const sensor_data_t* data);
bool microsd_read_last_line(const char* filename, char* buffer, size_t buffer_size);
bool microsd_file_exists(const char* filename);
uint32_t microsd_get_file_size(const char* filename);

// Funções de logging específicas para BioSmartCooler
bool microsd_start_logging(const char* filename);
bool microsd_log_sensor_data(const sensor_data_t* data);
bool microsd_log_event(const char* event_message);
bool microsd_log_alarm(const char* alarm_type, const char* description);

// Funções de utilidade
char* microsd_format_timestamp(time_t timestamp);
bool microsd_create_backup(const char* source_filename);
bool microsd_list_files(char* file_list, size_t max_size);
bool microsd_delete_file(const char* filename);

// Funções de configuração
void microsd_set_sample_interval(uint32_t interval_ms);
uint32_t microsd_get_sample_interval(void);
void microsd_set_auto_create(bool enable);
bool microsd_get_auto_create(void);

// Funções de diagnóstico
bool microsd_test_write(void);
bool microsd_test_read(void);
uint32_t microsd_get_free_space(void);
bool microsd_is_card_present(void);

// Função para obter total de registros
uint32_t microsd_get_total_records(void);

#endif // MICROSD_H 