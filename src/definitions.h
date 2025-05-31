#ifndef CENGINE_DEFINITIONS_H
#define CENGINE_DEFINITIONS_H
#pragma once

#include <asm-generic/errno.h>
#include <stdatomic.h>
#include <bits/pthreadtypes.h>

#define TICK_RATE 20

#define CONCAT(a, b) CONCAT_(a, b)
#define CONCAT_(a, b) a ## b

#define STR(a) #a
#define XSTR(a) STR(a)

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
