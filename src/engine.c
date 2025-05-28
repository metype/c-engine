#include "engine.h"
#include "actors/actor.h"

#include "util.h"
#include "definitions.h"

#include "log.h"

#include <malloc.h>
#include "errors.h"

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t thread_mutex_attr;

pthread_mutex_t actor_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t actor_list_mutex_attr;
bool thread_initialized = false;

thread_info* threads = nullptr;
actor* actor_list;

void *E_tick(__attribute__((unused)) void *arg)
{
    pthread_mutex_init(&actor_list_mutex, &actor_list_mutex_attr);
    E_start_thread();
    bool quit = false;
    pthread_mutex_lock(&actor_list_mutex);
    actor_list = A_make_actor(0, 0, A_get_actor_def("test_actor"));
    pthread_mutex_unlock(&actor_list_mutex);
    thread_info* this_thread = E_get_thread_info(pthread_self());
    while(!quit)
    {
        E_sleep(1.0f / TICK_RATE);

        pthread_mutex_lock(&actor_list_mutex);

        actor* cur_actor = actor_list;
        do {
            A_tick(cur_actor);
            cur_actor = cur_actor->next;
        } while(cur_actor);

        pthread_mutex_unlock(&actor_list_mutex);

        fflush(stdout);

        pthread_mutex_lock(E_get_thread_mutex());
            if(this_thread->status == THREAD_STOP_REQUESTED) {
                quit = true;
            }
        pthread_mutex_unlock(E_get_thread_mutex());
    }
    E_end_thread();
    return 0;
}

actor* E_get_actors() {
    pthread_mutex_lock(&actor_list_mutex);
    return actor_list;
}

void E_release_actors() {
    pthread_mutex_unlock(&actor_list_mutex);
}

thread_info* E_get_threads() {
    return threads;
}

thread_info* E_get_thread_info(pthread_t id) {
    pthread_mutex_lock(E_get_thread_mutex());
    assert(threads != nullptr, "Threads exist.");
    thread_info* current_thread = threads;
    do {
        if(current_thread->thread_id == id) {
            pthread_mutex_unlock(E_get_thread_mutex());
            return current_thread;
        }
        current_thread = current_thread->next;
    } while(current_thread->next);
    if(current_thread->thread_id == id) {
        pthread_mutex_unlock(E_get_thread_mutex());
        return current_thread;
    }
    pthread_mutex_unlock(E_get_thread_mutex());
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
    pthread_mutex_lock(E_get_thread_mutex());
    pthread_create(&tid, NULL, routine, arg);
    if(!threads) {
        threads = malloc(sizeof(thread_info));
        threads->status = THREAD_STARTING;
        threads->thread_id = tid;
        threads->next = nullptr;
        threads->prev = nullptr;
    } else {
        thread_info* new_thread = malloc(sizeof(thread_info));
        new_thread->status = THREAD_STARTING;
        new_thread->thread_id = tid;
        new_thread->next = threads;
        new_thread->prev = nullptr;
        threads = new_thread;
        new_thread->next->prev = new_thread;
    }
    pthread_mutex_unlock(E_get_thread_mutex());
    return tid;
}