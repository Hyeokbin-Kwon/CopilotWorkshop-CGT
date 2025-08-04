#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "constants.h"

// 문자열 유틸리티 함수들
void trim_whitespace(char *str);
void to_lowercase(char *str);
void to_uppercase(char *str);
int is_empty_string(const char *str);
int string_equals_ignore_case(const char *str1, const char *str2);
void safe_string_copy(char *dest, const char *src, size_t dest_size);
int is_valid_string_length(const char *str, int min_length, int max_length);

// 숫자 유틸리티 함수들
int is_valid_integer(const char *str);
int is_positive_integer(const char *str);
int parse_integer(const char *str, int *result);
int is_in_range(int value, int min, int max);

// 날짜/시간 유틸리티 함수들
time_t string_to_time(const char *date_string, const char *format);
void time_to_string(time_t time_val, char *buffer, size_t buffer_size, const char *format);
int is_valid_date_format(const char *date_string, const char *format);
time_t add_days_to_time(time_t base_time, int days);
int get_days_difference(time_t start_time, time_t end_time);
int is_future_date(time_t date);
int is_past_date(time_t date);

// 메모리 관리 유틸리티 함수들
void* safe_malloc(size_t size);
void* safe_realloc(void *ptr, size_t size);
void safe_free(void **ptr);

// 파일 I/O 유틸리티 함수들
int file_exists(const char *filename);
long get_file_size(const char *filename);
int create_directory_if_not_exists(const char *dir_path);
int backup_file(const char *source_path, const char *backup_path);

// 사용자 입력 유틸리티 함수들
int get_user_input(char *buffer, size_t buffer_size, const char *prompt);
int get_integer_input(int *result, const char *prompt, int min_value, int max_value);
int get_yes_no_input(const char *prompt);
int get_menu_choice(int min_choice, int max_choice, const char *prompt);

// 데이터 검증 유틸리티 함수들
int is_valid_email(const char *email);
int is_valid_phone(const char *phone);
int is_valid_isbn(const char *isbn);
int validate_book_data(const char *title, const char *author, const char *isbn);
int validate_member_data(const char *name, const char *email, const char *phone);

// 배열/리스트 유틸리티 함수들
void int_array_sort(int *array, int size, int ascending);
int int_array_search(const int *array, int size, int target);
void shuffle_int_array(int *array, int size);

// 보안 유틸리티 함수들
void generate_random_string(char *buffer, size_t length);
int hash_string(const char *str);
int compare_strings_secure(const char *str1, const char *str2);

// 로깅 유틸리티 함수들
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

int init_logging(const char *log_file_path);
void log_message(LogLevel level, const char *format, ...);
void close_logging(void);

// 설정 관리 유틸리티 함수들
typedef struct {
    char database_path[MAX_PATH_LENGTH];
    char backup_directory[MAX_PATH_LENGTH];
    int default_loan_days;
    int max_loan_count;
    int max_renewal_count;
    int auto_backup_enabled;
    int log_level;
} SystemConfig;

int load_config(const char *config_file, SystemConfig *config);
int save_config(const char *config_file, const SystemConfig *config);
void init_default_config(SystemConfig *config);

// 성능 측정 유틸리티 함수들
typedef struct {
    clock_t start_time;
    clock_t end_time;
    double elapsed_time;
} Timer;

void timer_start(Timer *timer);
void timer_stop(Timer *timer);
double timer_get_elapsed_seconds(const Timer *timer);
double timer_get_elapsed_milliseconds(const Timer *timer);

// 문자열 포맷팅 유틸리티 함수들
int format_size_string(long bytes, char *buffer, size_t buffer_size);
int format_number_with_commas(long number, char *buffer, size_t buffer_size);
int format_percentage(double value, char *buffer, size_t buffer_size);

// 통계 유틸리티 함수들
double calculate_average(const int *values, int count);
int find_median(const int *values, int count);
int find_mode(const int *values, int count);
int find_min_value(const int *values, int count);
int find_max_value(const int *values, int count);

// 색상 출력 유틸리티 함수들 (콘솔)
void print_colored_text(const char *text, const char *color);
void print_success_message(const char *message);
void print_error_message(const char *message);
void print_warning_message(const char *message);
void print_info_message(const char *message);

// 진행률 표시 유틸리티 함수들
void print_progress_bar(int current, int total, int width);
void clear_line(void);
void move_cursor_up(int lines);

#endif // UTILS_H
