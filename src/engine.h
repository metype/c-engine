#ifndef CPROJ_ENGINE_H
#define CPROJ_ENGINE_H

#define TICK_RATE 20

#include <pthread.h>

enum THREAD_STATUS {
    THREAD_STARTING,
    THREAD_RUNNING,
    THREAD_STOPPING,
    THREAD_STOPPED,
};

typedef struct thread_info {
    pthread_t thread_id;
    int status;
    struct thread_info* next;
    struct thread_info* prev;
} thread_info;

void *E_tick(__attribute__((unused)) void *arg);

void E_end_thread();
void E_start_thread();

thread_info* E_get_thread_info(pthread_t id);

pthread_mutex_t* E_get_thread_mutex();

pthread_t E_spawn_thread(void * (*routine)(void *));

thread_info* E_get_threads();

#endif //CPROJ_ENGINE_H
