#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include "../include/utils.h"
#include "../include/constants.h"

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

// 전역 변수들
static FILE *log_file = NULL;
static SystemConfig current_config;

// 문자열 유틸리티 함수들
void trim_whitespace(char *str) {
    if (!str) return;
    
    // 앞쪽 공백 제거
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    
    // 뒤쪽 공백 제거
    char *end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    // 결과 복사
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

void to_lowercase(char *str) {
    if (!str) return;
    
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

void to_uppercase(char *str) {
    if (!str) return;
    
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int is_empty_string(const char *str) {
    if (!str) return TRUE;
    
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return FALSE;
        }
        str++;
    }
    return TRUE;
}

int string_equals_ignore_case(const char *str1, const char *str2) {
    if (!str1 || !str2) return FALSE;
    
    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) {
            return FALSE;
        }
        str1++;
        str2++;
    }
    
    return *str1 == *str2;
}

void safe_string_copy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) return;
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

int is_valid_string_length(const char *str, int min_length, int max_length) {
    if (!str) return FALSE;
    
    int len = strlen(str);
    return len >= min_length && len <= max_length;
}

// 숫자 유틸리티 함수들
int is_valid_integer(const char *str) {
    if (!str || *str == '\0') return FALSE;
    
    // 부호 처리
    if (*str == '+' || *str == '-') str++;
    
    // 숫자 확인
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return FALSE;
        }
        str++;
    }
    
    return TRUE;
}

int is_positive_integer(const char *str) {
    if (!is_valid_integer(str)) return FALSE;
    
    int value = atoi(str);
    return value > 0;
}

int parse_integer(const char *str, int *result) {
    if (!str || !result) return FAILURE;
    
    if (!is_valid_integer(str)) return FAILURE;
    
    *result = atoi(str);
    return SUCCESS;
}

int is_in_range(int value, int min, int max) {
    return value >= min && value <= max;
}

// 날짜/시간 유틸리티 함수들
time_t string_to_time(const char *date_string, const char *format) {
    if (!date_string || !format) return 0;
    
    struct tm tm_struct = {0};
    
    // 기본적인 날짜 형식 파싱 (YYYY-MM-DD)
    if (strcmp(format, "%Y-%m-%d") == 0) {
        if (sscanf(date_string, "%d-%d-%d", 
                   &tm_struct.tm_year, &tm_struct.tm_mon, &tm_struct.tm_mday) == 3) {
            tm_struct.tm_year -= 1900;  // 1900년부터 시작
            tm_struct.tm_mon -= 1;      // 0부터 시작
            return mktime(&tm_struct);
        }
    }
    
    return 0;
}

void time_to_string(time_t time_val, char *buffer, size_t buffer_size, const char *format) {
    if (!buffer || buffer_size == 0) return;
    
    struct tm *tm_info = localtime(&time_val);
    if (!tm_info) {
        buffer[0] = '\0';
        return;
    }
    
    strftime(buffer, buffer_size, format ? format : "%Y-%m-%d %H:%M:%S", tm_info);
}

int is_valid_date_format(const char *date_string, const char *format) {
    return string_to_time(date_string, format) != 0;
}

time_t add_days_to_time(time_t base_time, int days) {
    return base_time + (days * 24 * 60 * 60);
}

int get_days_difference(time_t start_time, time_t end_time) {
    double diff_seconds = difftime(end_time, start_time);
    return (int)(diff_seconds / (24 * 60 * 60));
}

int is_future_date(time_t date) {
    return date > time(NULL);
}

int is_past_date(time_t date) {
    return date < time(NULL);
}

// 메모리 관리 유틸리티 함수들
void* safe_malloc(size_t size) {
    if (size == 0) return NULL;
    
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "메모리 할당 실패: %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    
    memset(ptr, 0, size);
    return ptr;
}

void* safe_realloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        fprintf(stderr, "메모리 재할당 실패: %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    
    return new_ptr;
}

void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

// 파일 I/O 유틸리티 함수들
int file_exists(const char *filename) {
    if (!filename) return FALSE;
    
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}

long get_file_size(const char *filename) {
    if (!filename) return -1;
    
    FILE *file = fopen(filename, "rb");
    if (!file) return -1;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    return size;
}

int create_directory_if_not_exists(const char *dir_path) {
    if (!dir_path) return FAILURE;
    
#ifdef _WIN32
    return _mkdir(dir_path) == 0 || errno == EEXIST ? SUCCESS : FAILURE;
#else
    return mkdir(dir_path, 0755) == 0 || errno == EEXIST ? SUCCESS : FAILURE;
#endif
}

int backup_file(const char *source_path, const char *backup_path) {
    if (!source_path || !backup_path) return FAILURE;
    
    FILE *source = fopen(source_path, "rb");
    if (!source) return FAILURE;
    
    FILE *backup = fopen(backup_path, "wb");
    if (!backup) {
        fclose(source);
        return FAILURE;
    }
    
    char buffer[4096];
    size_t bytes;
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes, backup) != bytes) {
            fclose(source);
            fclose(backup);
            return FAILURE;
        }
    }
    
    fclose(source);
    fclose(backup);
    return SUCCESS;
}

// 사용자 입력 유틸리티 함수들
int get_user_input(char *buffer, size_t buffer_size, const char *prompt) {
    if (!buffer || buffer_size == 0) return FAILURE;
    
    if (prompt) {
        printf("%s", prompt);
    }
    
    if (fgets(buffer, buffer_size, stdin)) {
        // 개행 문자 제거
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        trim_whitespace(buffer);
        return SUCCESS;
    }
    
    return FAILURE;
}

int get_integer_input(int *result, const char *prompt, int min_value, int max_value) {
    if (!result) return FAILURE;
    
    char input[32];
    
    while (1) {
        if (get_user_input(input, sizeof(input), prompt) != SUCCESS) {
            continue;
        }
        
        if (parse_integer(input, result) == SUCCESS) {
            if (is_in_range(*result, min_value, max_value)) {
                return SUCCESS;
            } else {
                printf("값은 %d와 %d 사이여야 합니다.\n", min_value, max_value);
            }
        } else {
            printf("유효한 숫자를 입력해주세요.\n");
        }
    }
}

int get_yes_no_input(const char *prompt) {
    char input[16];
    
    while (1) {
        if (get_user_input(input, sizeof(input), prompt) == SUCCESS) {
            to_lowercase(input);
            
            if (strcmp(input, "y") == 0 || strcmp(input, "yes") == 0 || 
                strcmp(input, "예") == 0 || strcmp(input, "네") == 0) {
                return TRUE;
            } else if (strcmp(input, "n") == 0 || strcmp(input, "no") == 0 || 
                      strcmp(input, "아니오") == 0 || strcmp(input, "아니") == 0) {
                return FALSE;
            } else {
                printf("'y/yes/예/네' 또는 'n/no/아니오/아니'로 답해주세요.\n");
            }
        }
    }
}

int get_menu_choice(int min_choice, int max_choice, const char *prompt) {
    int choice;
    char full_prompt[256];
    
    snprintf(full_prompt, sizeof(full_prompt), "%s (%d-%d): ", prompt, min_choice, max_choice);
    
    return get_integer_input(&choice, full_prompt, min_choice, max_choice) == SUCCESS ? choice : -1;
}

// 데이터 검증 유틸리티 함수들
int is_valid_email(const char *email) {
    if (!email || is_empty_string(email)) return FALSE;
    
    // 기본적인 이메일 형식 검증
    const char *at = strchr(email, '@');
    if (!at) return FALSE;
    
    const char *dot = strrchr(at, '.');
    if (!dot || dot <= at + 1) return FALSE;
    
    // 길이 확인
    if (!is_valid_string_length(email, 5, MAX_EMAIL_LENGTH)) return FALSE;
    
    return TRUE;
}

int is_valid_phone(const char *phone) {
    if (!phone || is_empty_string(phone)) return FALSE;
    
    // 숫자와 하이픈만 허용
    for (int i = 0; phone[i]; i++) {
        if (!isdigit((unsigned char)phone[i]) && phone[i] != '-') {
            return FALSE;
        }
    }
    
    return is_valid_string_length(phone, 10, MAX_PHONE_LENGTH);
}

int is_valid_isbn(const char *isbn) {
    if (!isbn || is_empty_string(isbn)) return FALSE;
    
    // 기본 길이 확인 (ISBN-10 또는 ISBN-13)
    int len = strlen(isbn);
    if (len != 10 && len != 13 && len != 17) return FALSE; // 17은 하이픈 포함
    
    // ISBN 형식 기본 검증 (간단한 버전)
    int digit_count = 0;
    for (int i = 0; isbn[i]; i++) {
        if (isdigit((unsigned char)isbn[i]) || (isbn[i] == 'X' && i == len - 1)) {
            digit_count++;
        } else if (isbn[i] != '-') {
            return FALSE;
        }
    }
    
    return digit_count == 10 || digit_count == 13;
}

int validate_book_data(const char *title, const char *author, const char *isbn) {
    if (is_empty_string(title) || is_empty_string(author)) {
        return FAILURE;
    }
    
    if (isbn && !is_empty_string(isbn) && !is_valid_isbn(isbn)) {
        return FAILURE;
    }
    
    return SUCCESS;
}

int validate_member_data(const char *name, const char *email, const char *phone) {
    if (is_empty_string(name)) {
        return FAILURE;
    }
    
    if (!is_valid_email(email)) {
        return FAILURE;
    }
    
    if (phone && !is_empty_string(phone) && !is_valid_phone(phone)) {
        return FAILURE;
    }
    
    return SUCCESS;
}

// 배열/리스트 유틸리티 함수들
void int_array_sort(int *array, int size, int ascending) {
    if (!array || size <= 1) return;
    
    // 간단한 버블 정렬
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            int should_swap = ascending ? (array[j] > array[j + 1]) : (array[j] < array[j + 1]);
            
            if (should_swap) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

int int_array_search(const int *array, int size, int target) {
    if (!array) return -1;
    
    for (int i = 0; i < size; i++) {
        if (array[i] == target) {
            return i;
        }
    }
    
    return -1;
}

void shuffle_int_array(int *array, int size) {
    if (!array || size <= 1) return;
    
    srand((unsigned int)time(NULL));
    
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// 보안 유틸리티 함수들
void generate_random_string(char *buffer, size_t length) {
    if (!buffer || length == 0) return;
    
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static int seeded = 0;
    
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    for (size_t i = 0; i < length - 1; i++) {
        int index = rand() % (sizeof(charset) - 1);
        buffer[i] = charset[index];
    }
    buffer[length - 1] = '\0';
}

int hash_string(const char *str) {
    if (!str) return 0;
    
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return (int)hash;
}

int compare_strings_secure(const char *str1, const char *str2) {
    if (!str1 || !str2) return FALSE;
    
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    
    if (len1 != len2) return FALSE;
    
    int result = 0;
    for (int i = 0; i < len1; i++) {
        result |= str1[i] ^ str2[i];
    }
    
    return result == 0;
}

// 로깅 유틸리티 함수들
int init_logging(const char *log_file_path) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    
    if (log_file_path) {
        log_file = fopen(log_file_path, "a");
        if (!log_file) {
            fprintf(stderr, "로그 파일을 열 수 없습니다: %s\n", log_file_path);
            return FAILURE;
        }
    }
    
    return SUCCESS;
}

void log_message(LogLevel level, const char *format, ...) {
    if (!format) return;
    
    FILE *output = log_file ? log_file : stdout;
    
    time_t now = time(NULL);
    char time_str[64];
    time_to_string(now, time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S");
    
    const char *level_str;
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG"; break;
        case LOG_INFO:    level_str = "INFO"; break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR"; break;
        default:          level_str = "UNKNOWN"; break;
    }
    
    fprintf(output, "[%s] %s: ", time_str, level_str);
    
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
    
    fprintf(output, "\n");
    fflush(output);
}

void close_logging(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

// 설정 관리 유틸리티 함수들
int load_config(const char *config_file, SystemConfig *config) {
    if (!config_file || !config) return FAILURE;
    
    FILE *file = fopen(config_file, "r");
    if (!file) {
        init_default_config(config);
        return FAILURE;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        trim_whitespace(line);
        
        if (line[0] == '#' || line[0] == '\0') continue;
        
        char *equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;
        
        trim_whitespace(key);
        trim_whitespace(value);
        
        if (strcmp(key, "database_path") == 0) {
            safe_string_copy(config->database_path, value, sizeof(config->database_path));
        } else if (strcmp(key, "backup_directory") == 0) {
            safe_string_copy(config->backup_directory, value, sizeof(config->backup_directory));
        } else if (strcmp(key, "default_loan_days") == 0) {
            parse_integer(value, &config->default_loan_days);
        } else if (strcmp(key, "max_loan_count") == 0) {
            parse_integer(value, &config->max_loan_count);
        } else if (strcmp(key, "max_renewal_count") == 0) {
            parse_integer(value, &config->max_renewal_count);
        } else if (strcmp(key, "auto_backup_enabled") == 0) {
            config->auto_backup_enabled = (strcmp(value, "true") == 0) ? TRUE : FALSE;
        } else if (strcmp(key, "log_level") == 0) {
            parse_integer(value, &config->log_level);
        }
    }
    
    fclose(file);
    return SUCCESS;
}

int save_config(const char *config_file, const SystemConfig *config) {
    if (!config_file || !config) return FAILURE;
    
    FILE *file = fopen(config_file, "w");
    if (!file) return FAILURE;
    
    fprintf(file, "# Library Management System Configuration\n");
    fprintf(file, "database_path=%s\n", config->database_path);
    fprintf(file, "backup_directory=%s\n", config->backup_directory);
    fprintf(file, "default_loan_days=%d\n", config->default_loan_days);
    fprintf(file, "max_loan_count=%d\n", config->max_loan_count);
    fprintf(file, "max_renewal_count=%d\n", config->max_renewal_count);
    fprintf(file, "auto_backup_enabled=%s\n", config->auto_backup_enabled ? "true" : "false");
    fprintf(file, "log_level=%d\n", config->log_level);
    
    fclose(file);
    return SUCCESS;
}

void init_default_config(SystemConfig *config) {
    if (!config) return;
    
    safe_string_copy(config->database_path, "library.db", sizeof(config->database_path));
    safe_string_copy(config->backup_directory, "./backups", sizeof(config->backup_directory));
    config->default_loan_days = DEFAULT_LOAN_DAYS;
    config->max_loan_count = MAX_BOOKS_PER_MEMBER;
    config->max_renewal_count = MAX_RENEWAL_COUNT;
    config->auto_backup_enabled = TRUE;
    config->log_level = LOG_INFO;
}

// 성능 측정 유틸리티 함수들
void timer_start(Timer *timer) {
    if (!timer) return;
    
    timer->start_time = clock();
}

void timer_stop(Timer *timer) {
    if (!timer) return;
    
    timer->end_time = clock();
    timer->elapsed_time = ((double)(timer->end_time - timer->start_time)) / CLOCKS_PER_SEC;
}

double timer_get_elapsed_seconds(const Timer *timer) {
    if (!timer) return 0.0;
    
    return timer->elapsed_time;
}

double timer_get_elapsed_milliseconds(const Timer *timer) {
    if (!timer) return 0.0;
    
    return timer->elapsed_time * 1000.0;
}

// 문자열 포맷팅 유틸리티 함수들
int format_size_string(long bytes, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return FAILURE;
    
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = (double)bytes;
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%ld %s", bytes, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", size, units[unit_index]);
    }
    
    return SUCCESS;
}

int format_number_with_commas(long number, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return FAILURE;
    
    char temp[64];
    snprintf(temp, sizeof(temp), "%ld", number);
    
    int len = strlen(temp);
    int comma_count = (len - 1) / 3;
    
    if (buffer_size < len + comma_count + 1) return FAILURE;
    
    int src_idx = len - 1;
    int dst_idx = len + comma_count;
    buffer[dst_idx] = '\0';
    dst_idx--;
    
    int digit_count = 0;
    while (src_idx >= 0) {
        if (digit_count > 0 && digit_count % 3 == 0) {
            buffer[dst_idx--] = ',';
        }
        buffer[dst_idx--] = temp[src_idx--];
        digit_count++;
    }
    
    return SUCCESS;
}

int format_percentage(double value, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return FAILURE;
    
    snprintf(buffer, buffer_size, "%.1f%%", value * 100.0);
    return SUCCESS;
}

// 통계 유틸리티 함수들
double calculate_average(const int *values, int count) {
    if (!values || count <= 0) return 0.0;
    
    long sum = 0;
    for (int i = 0; i < count; i++) {
        sum += values[i];
    }
    
    return (double)sum / count;
}

int find_median(const int *values, int count) {
    if (!values || count <= 0) return 0;
    
    // 배열 복사 및 정렬
    int *sorted = safe_malloc(sizeof(int) * count);
    memcpy(sorted, values, sizeof(int) * count);
    int_array_sort(sorted, count, TRUE);
    
    int median;
    if (count % 2 == 0) {
        median = (sorted[count / 2 - 1] + sorted[count / 2]) / 2;
    } else {
        median = sorted[count / 2];
    }
    
    free(sorted);
    return median;
}

int find_mode(const int *values, int count) {
    if (!values || count <= 0) return 0;
    
    int mode = values[0];
    int max_count = 1;
    
    for (int i = 0; i < count; i++) {
        int current_count = 1;
        for (int j = i + 1; j < count; j++) {
            if (values[i] == values[j]) {
                current_count++;
            }
        }
        
        if (current_count > max_count) {
            max_count = current_count;
            mode = values[i];
        }
    }
    
    return mode;
}

int find_min_value(const int *values, int count) {
    if (!values || count <= 0) return 0;
    
    int min = values[0];
    for (int i = 1; i < count; i++) {
        if (values[i] < min) {
            min = values[i];
        }
    }
    
    return min;
}

int find_max_value(const int *values, int count) {
    if (!values || count <= 0) return 0;
    
    int max = values[0];
    for (int i = 1; i < count; i++) {
        if (values[i] > max) {
            max = values[i];
        }
    }
    
    return max;
}

// 색상 출력 유틸리티 함수들
void print_colored_text(const char *text, const char *color) {
    if (!text) return;
    
#ifdef _WIN32
    // Windows 콘솔 색상 지원
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;
    
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;
    
    if (strcmp(color, "red") == 0) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    } else if (strcmp(color, "green") == 0) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    } else if (strcmp(color, "yellow") == 0) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
    } else if (strcmp(color, "blue") == 0) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
    }
    
    printf("%s", text);
    SetConsoleTextAttribute(hConsole, saved_attributes);
#else
    // Unix/Linux 컬러 코드
    if (strcmp(color, "red") == 0) {
        printf("\033[31m%s\033[0m", text);
    } else if (strcmp(color, "green") == 0) {
        printf("\033[32m%s\033[0m", text);
    } else if (strcmp(color, "yellow") == 0) {
        printf("\033[33m%s\033[0m", text);
    } else if (strcmp(color, "blue") == 0) {
        printf("\033[34m%s\033[0m", text);
    } else {
        printf("%s", text);
    }
#endif
}

void print_success_message(const char *message) {
    if (!message) return;
    print_colored_text("✓ ", "green");
    print_colored_text(message, "green");
    printf("\n");
}

void print_error_message(const char *message) {
    if (!message) return;
    print_colored_text("✗ ", "red");
    print_colored_text(message, "red");
    printf("\n");
}

void print_warning_message(const char *message) {
    if (!message) return;
    print_colored_text("⚠ ", "yellow");
    print_colored_text(message, "yellow");
    printf("\n");
}

void print_info_message(const char *message) {
    if (!message) return;
    print_colored_text("ℹ ", "blue");
    print_colored_text(message, "blue");
    printf("\n");
}

// 진행률 표시 유틸리티 함수들
void print_progress_bar(int current, int total, int width) {
    if (total <= 0 || width <= 0) return;
    
    float progress = (float)current / total;
    int filled = (int)(progress * width);
    
    printf("\r[");
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            printf("█");
        } else {
            printf("░");
        }
    }
    printf("] %.1f%% (%d/%d)", progress * 100.0, current, total);
    fflush(stdout);
}

void clear_line(void) {
    printf("\r\033[K");
    fflush(stdout);
}

void move_cursor_up(int lines) {
    if (lines > 0) {
        printf("\033[%dA", lines);
        fflush(stdout);
    }
}
