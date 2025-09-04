#include "inc/microsd.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Stub implementations for all functions
bool microsd_init(void) { return true; }
void microsd_deinit(void) {}
bool microsd_is_mounted(void) { return true; }
bool microsd_create_file(const char* filename) { return true; }
bool microsd_write_header(const char* filename) { return true; }
bool microsd_append_data(const char* filename, const sensor_data_t* data) { return true; }
bool microsd_read_last_line(const char* filename, char* buffer, size_t buffer_size) { return false; }
bool microsd_file_exists(const char* filename) { return true; }
uint32_t microsd_get_file_size(const char* filename) { return 0; }
bool microsd_start_logging(const char* filename) { return true; }
bool microsd_log_sensor_data(const sensor_data_t* data) { return true; }
bool microsd_log_event(const char* event_message) { return true; }
bool microsd_log_alarm(const char* alarm_type, const char* description) { return true; }
char* microsd_format_timestamp(time_t timestamp) { return "2024-01-01 00:00:00"; }
bool microsd_create_backup(const char* source_filename) { return true; }
bool microsd_list_files(char* file_list, size_t max_size) { return true; }
bool microsd_delete_file(const char* filename) { return true; }
void microsd_set_sample_interval(uint32_t interval_ms) {}
uint32_t microsd_get_sample_interval(void) { return 5000; }
void microsd_set_auto_create(bool enable) {}
bool microsd_get_auto_create(void) { return true; }
bool microsd_test_write(void) { return true; }
bool microsd_test_read(void) { return true; }
uint32_t microsd_get_free_space(void) { return 1024 * 1024; }
bool microsd_is_card_present(void) { return true; }
uint32_t microsd_get_total_records(void) { return 0; } 