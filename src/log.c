#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "log.h"
#include "filesystem.h"
#include <pthread.h>

uint8_t current_log_level = ~0;
file_s log_file;

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

    char log_level_names[5][8] = {"INFO", "WARNING", "ERROR", "FATAL", "DEBUG"};

    snprintf(buf, buf_len, "[%s] [%s] %s", date_string, log_level_names[(int)log2(log_level)], str);

    size_t len = strlen(buf);
    if (buf[len - 1] != '\n') {
        strcat(buf, "\n");
    }

    free(date_string);

    return buf;
}

void Log_generic_log(uint8_t log_level, const char* str) {
    if(!(log_level & current_log_level)) return;
    // Print it to stdout
    fputs(str, stdout);

    // Lock the mutex, since we plan on using the log_file
    if(pthread_mutex_trylock(&log_mutex) == 0) {

        // Print the log line to the log_file and flush it.
        fputs(str, log_file.f_ptr);
        fflush(log_file.f_ptr);

        // Unlock the mutex since we're done
        pthread_mutex_unlock(&log_mutex);
    }
}

void Log_print(uint8_t log_level, const char* str) {
    if(!(log_level & current_log_level)) return;
    // Format the line
    char* buf = format_log_line(log_level, str);

    Log_generic_log(log_level, buf);

    // And clean up after ourselves
    free(buf);
}

void Log_printf(uint8_t log_level, const char* format, ...) {
    if(!(log_level & current_log_level)) return;

    va_list args;
    va_start(args, format);

    char* final_buf = malloc(sizeof(char) * 4096);

    // Format this log line
    char* buf = format_log_line(log_level, format);

    // Put in whatever data they need
    vsnprintf(final_buf, 4096, buf, args);

    // Clean up
    free(buf);

    va_end(args);

    // And log this fully formatted line
    Log_generic_log(log_level, final_buf);
    free(final_buf);
}

void Log_set_log_level(uint8_t log_level) {
    current_log_level = log_level;
}

void Log_init() {
    pthread_mutex_init(&log_mutex, &log_mutex_attr);
    pthread_mutex_lock(&log_mutex);

    log_file = FS_open_dated("/logs/%s.log", nullptr, FILE_WRITE);

    pthread_mutex_unlock(&log_mutex);

    Log_print(LOG_LEVEL_INFO, "Logger has been initialized.");
}

void Log_quit() {
    Log_print(LOG_LEVEL_INFO, "Logger has been de-initialized.");
    pthread_mutex_lock(&log_mutex);
    FS_close(log_file);
    pthread_mutex_unlock(&log_mutex);
}