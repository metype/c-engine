#include "engine.h"
#include "actors/actor.h"
#include "actors/test_actor.h"

#include "util.h"

#include <malloc.h>
#include "errors.h"
#include "app_state.h"

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t thread_mutex_attr;
bool thread_initialized = false;

thread_info* threads = nullptr;

void *E_tick(void *arg)
{
    E_start_thread();
    app_state* state_ptr = (app_state*)arg;
    bool quit = false;
    actor* root_actor = A_make_actor(0, 0, &test_actor_think);
    thread_info* this_thread = E_get_thread_info(pthread_self());
    while(!quit)
    {
        E_sleep(1.0f / TICK_RATE);

        actor* cur_actor = root_actor;
        do {
            A_tick(cur_actor, state_ptr);
            cur_actor = cur_actor->next;
        } while(cur_actor);

        fflush(stdout);
        if(this_thread->status == THREAD_STOP_REQUESTED) {
            quit = true;
        }
    }
    E_end_thread();
    return 0;
}

thread_info* E_get_threads() {
    return threads;
}

thread_info* E_get_thread_info(pthread_t id) {
    assert(threads != nullptr, "Threads exist.");
    thread_info* current_thread = threads;
    do {
        if(current_thread->thread_id == id) {
            return current_thread;
        }
        current_thread = current_thread->next;
    } while(current_thread->next);
    if(current_thread->thread_id == id) {
        return current_thread;
    }
    return nullptr;
}

void E_start_thread() {
    thread_info* info = E_get_thread_info(pthread_self());
    info->status = THREAD_RUNNING;
}

void E_end_thread() {
    thread_info* info = E_get_thread_info(pthread_self());
    if(info->status == THREAD_STOPPING) {
        info->status = THREAD_STOPPED;
        return;
    }
    info->status = THREAD_STOPPING;
}

pthread_mutex_t* E_get_thread_mutex() {
    if(!thread_initialized) {
        pthread_mutex_init(&thread_mutex, &thread_mutex_attr);
        thread_initialized = true;
    }
    return &thread_mutex;
}

pthread_t E_spawn_thread(void * (*routine)(void *), void* arg) {
    pthread_t tid;
    pthread_create(&tid, NULL, routine, arg);
    pthread_mutex_lock(E_get_thread_mutex());
    if(!threads) {
        threads = malloc(sizeof(thread_info));
        threads->status = THREAD_STARTING;
        threads->thread_id = tid;
        threads->next = nullptr;
        threads->prev = nullptr;
    } else {
        thread_info* current_thread;
        do {
            current_thread = threads->next;
        } while(current_thread->next);
        thread_info* new_thread = malloc(sizeof(thread_info));
        new_thread->status = THREAD_STARTING;
        new_thread->thread_id = tid;
        new_thread->next = nullptr;
        new_thread->prev = current_thread;
        current_thread->next = new_thread;
    }
    pthread_mutex_unlock(E_get_thread_mutex());
    return tid;
}