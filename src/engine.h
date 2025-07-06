#ifndef CPROJ_ENGINE_H
#define CPROJ_ENGINE_H

#include <pthread.h>
#include <stdio.h>
#include "SDL3/SDL_events.h"

struct actor;

enum THREAD_STATUS {
    THREAD_STARTING,
    THREAD_RUNNING,
    THREAD_STOPPING,
    THREAD_STOPPED,
    THREAD_STOP_REQUESTED,
    THREAD_PAUSED,
};

enum USER_EVENT_TYPE {
    USER_EVENT_THREAD_STOPPED,
    USER_EVENT_THREAD_STOPPING,
};

enum THREAD_TYPE {
    THREAD_TYPE_TICK,
    THREAD_TYPE_RENDER,
    THREAD_TYPE_SOUND,
    THREAD_TYPE_MISC,
};

typedef struct thread_info_s {
    pthread_t thread_id;
    int status;
    int type;
    struct thread_info_s* next;
    struct thread_info_s* prev;
} thread_info_s;

typedef struct app_state app_state;

void *Engine_tick(void *arg);
void Engine_event(struct app_state* state_ptr, SDL_Event* event);

void Engine_end_thread();
void Engine_start_thread();
void Engine_pause_thread(pthread_t id);
void Engine_pause_all_threads();
void Engine_unpause_thread(pthread_t id);
void Engine_unpause_all_threads();

float Engine_get_tick_frate();
int Engine_get_tick_rate();

thread_info_s* Engine_get_thread_info(pthread_t id);

pthread_mutex_t* Engine_get_thread_mutex();
pthread_mutex_t* Engine_get_actor_mutex();

pthread_t Engine_spawn_thread(void * (*routine)(void *), void* arg);

thread_info_s* Engine_get_threads();

struct actor_s* Engine_get_actors();

int Engine_set_real_tickrate(int tickrate_pref);

void Engine_get_memory(int* currRealMem, int* peakRealMem, int* currVirtMem, int* peakVirtMem);

#endif //CPROJ_ENGINE_H
