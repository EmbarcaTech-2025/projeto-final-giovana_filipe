#include "inc/microsd.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"        // FATFS
#include "diskio.h"    // FATFS low level disk I/O
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Variáveis globais
static FATFS fs;          // Estrutura FATFS
static bool fs_montado = false;
static logging_config_t logging_config = {
    .filename = "biosmartcooler.csv",
    .auto_create = true,
    .sample_interval_ms = 5000,  // 5 segundos
    .header_written = false
};

// Buffer para formatação de strings
static char timestamp_buffer[32];
static char line_buffer[MAX_LINE_LEN];

// Inicializa SPI e monta o sistema de arquivos
bool microsd_init(void) {
    // Inicializa SPI
    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz para inicialização
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1); // CS inativo

    // Monta filesystem
    FRESULT res = f_mount(&fs, "", 1);
    if (res == FR_OK) {
        fs_montado = true;
        return true;
    } else {
        fs_montado = false;
        return false;
    }
}

void microsd_deinit(void) {
    if (fs_montado) {
        f_unmount("");
        fs_montado = false;
    }
}

bool microsd_is_mounted(void) {
    return fs_montado;
}

// Cria um novo arquivo
bool microsd_create_file(const char* filename) {
    if (!fs_montado) return false;

    FIL file;
    FRESULT res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
        f_close(&file);
        return true;
    }
    return false;
}

// Escreve cabeçalho CSV
bool microsd_write_header(const char* filename) {
    if (!fs_montado) return false;

    FIL file;
    FRESULT res = f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) return false;

    UINT bytes_written;
    res = f_write(&file, CSV_HEADER, strlen(CSV_HEADER), &bytes_written);
    f_close(&file);

    return (res == FR_OK && bytes_written == strlen(CSV_HEADER));
}

// Adiciona dados dos sensores ao arquivo
bool microsd_append_data(const char* filename, const sensor_data_t* data) {
    if (!fs_montado || !data) return false;

    FIL file;
    FRESULT res = f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) return false;

    // Formata linha CSV
    snprintf(line_buffer, MAX_LINE_LEN,
             "%s,%.2f,%.2f,%d,%d,%d,%d,%d\n",
             microsd_format_timestamp(data->timestamp),
             data->temperatura,
             data->pressao,
             data->luminosidade,
             data->aceleracao_x,
             data->aceleracao_y,
             data->aceleracao_z,
             data->umidade);

    UINT bytes_written;
    res = f_write(&file, line_buffer, strlen(line_buffer), &bytes_written);
    f_close(&file);

    return (res == FR_OK && bytes_written == strlen(line_buffer));
}

// Lê a última linha do arquivo
bool microsd_read_last_line(const char* filename, char* buffer, size_t buffer_size) {
    if (!fs_montado || !buffer) return false;

    FIL file;
    FRESULT res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) return false;

    // Move para o final do arquivo
    f_lseek(&file, f_size(&file));
    
    // Procura pela última linha
    char temp_buffer[256];
    UINT bytes_read;
    bool found_line = false;
    
    // Lê de trás para frente até encontrar '\n'
    for (int i = 0; i < 1000; i++) { // Limite de segurança
        if (f_tell(&file) == 0) break;
        
        f_lseek(&file, f_tell(&file) - 1);
        f_read(&file, temp_buffer, 1, &bytes_read);
        
        if (temp_buffer[0] == '\n') {
            found_line = true;
            break;
        }
    }
    
    if (found_line) {
        // Lê a linha
        f_read(&file, buffer, buffer_size - 1, &bytes_read);
        buffer[bytes_read] = '\0';
    }
    
    f_close(&file);
    return found_line;
}

// Verifica se arquivo existe
bool microsd_file_exists(const char* filename) {
    if (!fs_montado) return false;

    FILINFO fno;
    FRESULT res = f_stat(filename, &fno);
    return (res == FR_OK);
}

// Obtém tamanho do arquivo
uint32_t microsd_get_file_size(const char* filename) {
    if (!fs_montado) return 0;

    FILINFO fno;
    FRESULT res = f_stat(filename, &fno);
    if (res == FR_OK) {
        return fno.fsize;
    }
    return 0;
}

// Inicia logging com arquivo específico
bool microsd_start_logging(const char* filename) {
    if (!fs_montado) return false;

    strncpy(logging_config.filename, filename, MAX_FILENAME_LEN - 1);
    logging_config.filename[MAX_FILENAME_LEN - 1] = '\0';
    
    // Cria arquivo se não existir
    if (!microsd_file_exists(filename)) {
        if (!microsd_create_file(filename)) return false;
        if (!microsd_write_header(filename)) return false;
    }
    
    logging_config.header_written = true;
    return true;
}

// Log de dados dos sensores
bool microsd_log_sensor_data(const sensor_data_t* data) {
    return microsd_append_data(logging_config.filename, data);
}

// Log de eventos
bool microsd_log_event(const char* event_message) {
    if (!fs_montado) return false;

    FIL file;
    FRESULT res = f_open(&file, "events.log", FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) return false;

    // Formata linha de evento
    snprintf(line_buffer, MAX_LINE_LEN, "%s - EVENT: %s\n",
             microsd_format_timestamp(time(NULL)), event_message);

    UINT bytes_written;
    res = f_write(&file, line_buffer, strlen(line_buffer), &bytes_written);
    f_close(&file);

    return (res == FR_OK);
}

// Log de alarmes
bool microsd_log_alarm(const char* alarm_type, const char* description) {
    if (!fs_montado) return false;

    FIL file;
    FRESULT res = f_open(&file, "alarms.log", FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) return false;

    // Formata linha de alarme
    snprintf(line_buffer, MAX_LINE_LEN, "%s - ALARM [%s]: %s\n",
             microsd_format_timestamp(time(NULL)), alarm_type, description);

    UINT bytes_written;
    res = f_write(&file, line_buffer, strlen(line_buffer), &bytes_written);
    f_close(&file);

    return (res == FR_OK);
}

// Formata timestamp
char* microsd_format_timestamp(time_t timestamp) {
    struct tm* tm_info = localtime(&timestamp);
    strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return timestamp_buffer;
}

// Cria backup do arquivo
bool microsd_create_backup(const char* source_filename) {
    if (!fs_montado) return false;

    char backup_filename[MAX_FILENAME_LEN];
    snprintf(backup_filename, MAX_FILENAME_LEN, "%s.bak", source_filename);

    FIL source_file, backup_file;
    FRESULT res = f_open(&source_file, source_filename, FA_READ);
    if (res != FR_OK) return false;

    res = f_open(&backup_file, backup_filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        f_close(&source_file);
        return false;
    }

    // Copia arquivo
    char buffer[512];
    UINT bytes_read, bytes_written;
    bool success = true;

    while (true) {
        res = f_read(&source_file, buffer, sizeof(buffer), &bytes_read);
        if (res != FR_OK || bytes_read == 0) break;

        res = f_write(&backup_file, buffer, bytes_read, &bytes_written);
        if (res != FR_OK || bytes_written != bytes_read) {
            success = false;
            break;
        }
    }

    f_close(&source_file);
    f_close(&backup_file);
    return success;
}

// Lista arquivos
bool microsd_list_files(char* file_list, size_t max_size) {
    if (!fs_montado || !file_list) return false;

    DIR dir;
    FILINFO fno;
    FRESULT res = f_opendir(&dir, "");
    if (res != FR_OK) return false;

    file_list[0] = '\0';
    size_t current_len = 0;

    while (true) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;

        if (!(fno.fattrib & AM_DIR)) { // Apenas arquivos
            size_t name_len = strlen(fno.fname);
            if (current_len + name_len + 2 < max_size) {
                strcat(file_list, fno.fname);
                strcat(file_list, "\n");
                current_len += name_len + 1;
            }
        }
    }

    f_closedir(&dir);
    return true;
}

// Deleta arquivo
bool microsd_delete_file(const char* filename) {
    if (!fs_montado) return false;

    FRESULT res = f_unlink(filename);
    return (res == FR_OK);
}

// Configurações
void microsd_set_sample_interval(uint32_t interval_ms) {
    logging_config.sample_interval_ms = interval_ms;
}

uint32_t microsd_get_sample_interval(void) {
    return logging_config.sample_interval_ms;
}

void microsd_set_auto_create(bool enable) {
    logging_config.auto_create = enable;
}

bool microsd_get_auto_create(void) {
    return logging_config.auto_create;
}

// Testes de diagnóstico
bool microsd_test_write(void) {
    if (!fs_montado) return false;

    const char* test_filename = "test_write.txt";
    const char* test_data = "Teste de escrita microSD - BioSmartCooler\n";

    FIL file;
    FRESULT res = f_open(&file, test_filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) return false;

    UINT bytes_written;
    res = f_write(&file, test_data, strlen(test_data), &bytes_written);
    f_close(&file);

    if (res == FR_OK) {
        f_unlink(test_filename); // Remove arquivo de teste
        return true;
    }
    return false;
}

bool microsd_test_read(void) {
    if (!fs_montado) return false;

    const char* test_filename = "test_read.txt";
    const char* test_data = "Teste de leitura microSD - BioSmartCooler\n";

    // Cria arquivo de teste
    FIL file;
    FRESULT res = f_open(&file, test_filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) return false;

    UINT bytes_written;
    f_write(&file, test_data, strlen(test_data), &bytes_written);
    f_close(&file);

    // Testa leitura
    res = f_open(&file, test_filename, FA_READ);
    if (res != FR_OK) return false;

    char buffer[100];
    UINT bytes_read;
    res = f_read(&file, buffer, sizeof(buffer) - 1, &bytes_read);
    f_close(&file);

    if (res == FR_OK) {
        buffer[bytes_read] = '\0';
        f_unlink(test_filename); // Remove arquivo de teste
        return (strcmp(buffer, test_data) == 0);
    }
    return false;
}

uint32_t microsd_get_free_space(void) {
    if (!fs_montado) return 0;

    FATFS* fs_ptr;
    DWORD fre_clust;
    FRESULT res = f_getfree("", &fre_clust, &fs_ptr);
    if (res == FR_OK) {
        return (uint32_t)(fre_clust * fs_ptr->csize * 512); // Tamanho em bytes
    }
    return 0;
}

bool microsd_is_card_present(void) {
    // Verifica se o cartão está presente tentando montar o filesystem
    if (!fs_montado) {
        return microsd_init();
    }
    return true;
}

// Obtém o total de registros no arquivo de log
uint32_t microsd_get_total_records(void) {
    if (!fs_montado) return 0;
    
    FIL file;
    FRESULT res = f_open(&file, logging_config.filename, FA_READ);
    if (res != FR_OK) return 0;
    
    // Conta as linhas no arquivo (cada linha é um registro)
    uint32_t line_count = 0;
    char buffer[256];
    UINT bytes_read;
    
    // Pula o cabeçalho se existir
    f_gets(buffer, sizeof(buffer), &file);
    if (strstr(buffer, "Timestamp") != NULL) {
        // É o cabeçalho, não conta
    } else {
        // Primeira linha é um registro
        line_count = 1;
    }
    
    // Conta as demais linhas
    while (f_gets(buffer, sizeof(buffer), &file) != NULL) {
        line_count++;
    }
    
    f_close(&file);
    return line_count;
} 