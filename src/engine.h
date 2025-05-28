#ifndef CPROJ_ENGINE_H
#define CPROJ_ENGINE_H

#include <pthread.h>

struct actor;

enum THREAD_STATUS {
    THREAD_STARTING,
    THREAD_RUNNING,
    THREAD_STOPPING,
    THREAD_STOPPED,
    THREAD_STOP_REQUESTED,
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

pthread_t E_spawn_thread(void * (*routine)(void *), void* arg);

thread_info* E_get_threads();

struct actor* E_get_actors();

void E_release_actors();

#endif //CPROJ_ENGINE_H
