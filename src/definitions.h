#ifndef CENGINE_DEFINITIONS_H
#define CENGINE_DEFINITIONS_H
#pragma once


#if defined(__linux__)
#define CENGINE_LINUX 1
#elif defined(__WIN32) || defined(_WIN32_WINNT) || defined(_WIN32)
#define CENGINE_WIN32 1
#endif

#if defined(_MSC_VER)
#define CENGINE_MSVC 1
#else
#define CENGINE_GENCOMP 1
#endif

#if CENGINE_LINUX
    #include <asm-generic/errno.h>
#elif CENGINE_WIN32
    #include "../pthread-win32/pthread.h"
#endif

#include <stdatomic.h>

#define TICK_RATE 20

#define CONCAT(a, b) CONCAT_(a, b)
#define CONCAT_(a, b) a ## b

#define STR(a) #a
#define XSTR(a) STR(a)

#ifndef CENGINE_CALL
#if defined(CENGINE_WIN32) && !defined(__GNUC__)
#define CENGINE_CALL __cdecl
#else
#define CENGINE_CALL
#endif //defined(CENGINE_WIN32) && !defined(__GNUC__)
#endif //CENGINE_CALL

#define mutex_locked_code(mutex, expr) do {   \
    int err = pthread_mutex_trylock(mutex);   \
                                              \
    if(err == EDEADLK || err == 0) {          \
        expr                                  \
    }                                         \
                                              \
    if(err == 0) pthread_mutex_unlock(mutex); \
} while(0)

#endif //CENGINE_DEFINITIONS_H
