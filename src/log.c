#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <linux/limits.h>
#include "log.h"
#include "filesystem.h"
#include "string.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

uint8_t current_log_level = ~0;
FILE* log_file;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t log_mutex_attr;

char* format_log_line(uint8_t log_level, const char* str) {
    const int date_str_len = 20;
    const int buf_padding = 4;

    // Try to ensure we allocate exactly how much we need and no more, but also no less.
    unsigned long int buf_len = strlen(str) + date_str_len + buf_padding;

    time_t current_time = time(NULL);
    char* date_string = malloc(sizeof(char) * date_str_len);
    strftime(date_string, date_str_len, "%H:%M:%S", localtime(&current_time));

    char* buf = malloc(sizeof(char) * buf_len);

    if(strlen(str) + strlen(date_string) + buf_padding > buf_len) {
        free(buf);
        char* dire_warning = malloc(sizeof(char) * buf_len);
        dire_warning[0] = 0;
        strcat(dire_warning, "LOG LINE TOO LONG, DROPPED.\n");
        return dire_warning;
    }

    char log_level_names[5][7] = {"INFO", "WARNING", "ERROR", "FATAL", "DEBUG"};

    sprintf(buf, "[%s] [%s] %s", date_string, log_level_names[(int)log2(log_level)], str);
    size_t len = strlen(buf);
    if (buf[len - 1] != '\n') {
        strcat(buf, "\n");
    }

    free(date_string);

    return buf;
}

void L_print(uint8_t log_level, const char* str) {
    if(!(log_level & current_log_level)) return;
    char* buf = format_log_line(log_level, str);
    fputs(buf, stdout);
    pthread_mutex_lock(&log_mutex);
    fprintf(log_file, "%s", buf);
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);
    free(buf);
}

void L_printf(uint8_t log_level, const char* format, ...) {
    if(!(log_level & current_log_level)) return;
    va_list args;
    va_start(args, format);
    char* buf = format_log_line(log_level, format);
    vprintf(buf, args);
    va_start(args, format);
    pthread_mutex_lock(&log_mutex);
    vfprintf(log_file, buf, args);
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);
    free(buf);
    va_end(args);
}

void L_set_log_level(uint8_t log_level) {
    current_log_level = log_level;
}

void L_init() {
    pthread_mutex_init(&log_mutex, &log_mutex_attr);
    pthread_mutex_lock(&log_mutex);
    string* working_dir = sc(F_get_working_dir());
    s_cat(working_dir, so("/logs"));

    struct stat st = {0};

    if (stat(working_dir->c_str, &st) == -1) {
        mkdir(working_dir->c_str, 0700);
    }

    const int date_str_len = 20;

    time_t current_time = time(nullptr);
    char* date_string = malloc(sizeof(char) * date_str_len);
    strftime(date_string, date_str_len, "%Y-%j-%H-%M-%S", localtime(&current_time));

    s_cat(working_dir, so("/"));
    s_cat(working_dir, sco(date_string));
    s_cat(working_dir, so(".log"));

    log_file = fopen(working_dir->c_str, "w");
    pthread_mutex_unlock(&log_mutex);

    ref_dec(&working_dir->refcount);

    L_print(LOG_LEVEL_INFO, "Logger has been initialized.");
}

void L_quit() {
    L_print(LOG_LEVEL_INFO, "Logger has been de-initialized.");
    pthread_mutex_lock(&log_mutex);
    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
}