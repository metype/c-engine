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
    THREAD_PAUSED,
};

typedef struct thread_info_s {
    pthread_t thread_id;
    int status;
    struct thread_info_s* next;
    struct thread_info_s* prev;
} thread_info_s;

void *Engine_tick(__attribute__((unused)) void *arg);

void Engine_end_thread();
void Engine_start_thread();
void Engine_pause_thread(pthread_t id);
void Engine_pause_all_threads();
void Engine_unpause_thread(pthread_t id);
void Engine_unpause_all_threads();

int Engine_get_tick_rate();

thread_info_s* Engine_get_thread_info(pthread_t id);

pthread_mutex_t* Engine_get_thread_mutex();
pthread_mutex_t* Engine_get_actor_mutex();

pthread_t Engine_spawn_thread(void * (*routine)(void *), void* arg);

thread_info_s* Engine_get_threads();

struct actor_s* Engine_get_actors();

#endif //CPROJ_ENGINE_H
