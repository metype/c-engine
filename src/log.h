#ifndef CENGINE_LOG_H
#define CENGINE_LOG_H

#include <stdint.h>

typedef enum log_level : uint8_t {
    LOG_LEVEL_INFO = 0b00000001,
    LOG_LEVEL_WARNING = 0b00000010,
    LOG_LEVEL_ERROR = 0b00000100,
    LOG_LEVEL_FATAL = 0b00001000,
    LOG_LEVEL_DEBUG = 0b00010000,
} log_level;

void Log_init();
void Log_set_log_level(uint8_t log_level);
void Log_print(uint8_t log_level, const char* str);
void Log_printf(uint8_t log_level, const char* format, ...);
void Log_quit();

#define printf NOCALL

#endif //CENGINE_LOG_H
