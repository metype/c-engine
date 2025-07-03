#include "engine.h"
#include "actors/actor.h"
#include "scene.h"

#include "util.h"
#include "definitions.h"

#include "log.h"

#include <malloc.h>
#include "errors.h"
#include "rendering.h"
#include "actors/lua_script.h"
#include "filesystem.h"
#include "viewport.h"

#if defined(__linux__)
#include <sys/time.h>
#elif defined(__WIN32) || defined(_WIN32_WINNT)
#include "win32_stdlib.h"
#endif

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t thread_mutex_attr;
bool thread_mutex_initialized = false;

pthread_mutex_t actor_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t actor_list_mutex_attr;
bool actor_mutex_initialized = false;

thread_info_s* threads = nullptr;
actor_s* actor_list;
atomic_int effective_tick_rate = 0;

void *Engine_tick(void *arg)
{
    app_state_s* state_ptr = arg;

    state_ptr->perf_metrics_ptr->tick_rate = Engine_set_real_tickrate(state_ptr->perf_metrics_ptr->tick_rate);

    Register_base_serialization_types();
    Register_serialization_func("actor", &Actor_serialize);
    Register_deserialization_func("actor", &Actor_deserialize);

    Register_serialization_func("scene", &Scene_serialize);
    Register_deserialization_func("scene", &Scene_deserialize);

    Register_serialization_func("transform", &Transform_serialize);
    Register_deserialization_func("transform", &Transform_deserialize);

    Register_serialization_func("viewport", &Viewport_serialize);
    Register_deserialization_func("viewport", &Viewport_deserialize);

    pthread_mutexattr_init(&actor_list_mutex_attr);
    pthread_mutexattr_settype(&actor_list_mutex_attr, PTHREAD_MUTEX_DEFAULT);
    pthread_mutex_init(&actor_list_mutex, &actor_list_mutex_attr);

    actor_mutex_initialized = true;
    Engine_start_thread();
    bool quit = false;
    mutex_locked_code(&actor_list_mutex, {
        string* working_dir = sc(FS_get_working_dir());
        s_cat(working_dir, so("/saves"));
        FS_create_dir_if_not_exist(working_dir->c_str);
        s_cat(working_dir, so("/main.scene"));

        FILE* scene_file = fopen(working_dir->c_str, "r");
        scene* scene = Deserialize_file_to_obj(scene_file);

        Scene_save(scene, "test1");

        state_ptr->scene = scene;
        actor_list = (actor_s*) scene->actor_tree;

        ref_dec(&working_dir->refcount);
    });


    thread_info_s* this_thread = Engine_get_thread_info(pthread_self());

    while(!quit)
    {
        // Let's not fucking max out a core while we're paused lmao
        // This has the added benefit of keeping ticks time-consistent, which is not important, but nice
        if(this_thread->status == THREAD_PAUSED) { Thread_sleep(1.0f / (float) effective_tick_rate); continue; }

        struct timeval tp1;
        gettimeofday(&tp1, nullptr);
        long us1 = tp1.tv_sec * 1000000 + tp1.tv_usec;

        int aerr = pthread_mutex_trylock(Engine_get_thread_mutex());
        while(aerr == EBUSY) {
            aerr = pthread_mutex_trylock(Engine_get_thread_mutex());
        }
        mutex_locked_code(&actor_list_mutex, {
            Actor_update_transforms(actor_list);
            Actor_tick(actor_list, state_ptr);

            if(aerr == 0) {
                state_ptr->perf_metrics_ptr->tick_timer = 0;
                if (this_thread->status == THREAD_STOP_REQUESTED) {
                    quit = true;
                }
                pthread_mutex_unlock(Engine_get_thread_mutex());
            }
        });


        fflush(stdout);

        struct timeval tp2;
        gettimeofday(&tp2, nullptr);
        long us2 = tp2.tv_sec * 1000000 + tp2.tv_usec;

        float delta = (float)(us2 - us1);
        float wait_time = (1.0f / (float) effective_tick_rate) - (delta * 0.000001f);

        if(wait_time > 0) Thread_sleep(wait_time);
    }
    Engine_end_thread();
    return 0;
}

void Engine_event(app_state* state_ptr, SDL_Event* event) {
    mutex_locked_code(&actor_list_mutex, {
        Actor_event(actor_list, state_ptr, event);
    });
}

float Engine_get_tick_frate() {
    return 1.f / (float)Engine_get_tick_rate();
}

int Engine_get_tick_rate() {
    return effective_tick_rate;
}

void Engine_pause_thread(pthread_t id) {
    thread_info_s *info = Engine_get_thread_info(id);
    mutex_locked_code(Engine_get_thread_mutex(), info->status = THREAD_PAUSED;);
}

void Engine_pause_all_threads() {
    mutex_locked_code(Engine_get_thread_mutex(), {
        assert(threads != nullptr, "Threads exist.", return);
        thread_info_s* current_thread = threads;
        do {
            Engine_pause_thread(current_thread->thread_id);
            current_thread = current_thread->next;
        } while(current_thread);
    });
}

void Engine_unpause_thread(pthread_t id) {
    thread_info_s *info = Engine_get_thread_info(id);

    mutex_locked_code(Engine_get_thread_mutex(), info->status = THREAD_RUNNING;);
}

void Engine_unpause_all_threads() {
    mutex_locked_code(Engine_get_thread_mutex(), {
        assert(threads != nullptr, "Threads exist.", return);
        thread_info_s* current_thread = threads;
        do {
            Engine_unpause_thread(current_thread->thread_id);
            current_thread = current_thread->next;
        } while(current_thread);
    });
}

actor_s* Engine_get_actors() {
    return actor_list;
}

thread_info_s* Engine_get_threads() {
    return threads;
}

thread_info_s* Engine_get_thread_info(pthread_t id) {
#if CENGINE_LINUX
    mutex_locked_code(Engine_get_thread_mutex(), {
        assert(threads != nullptr, "Threads exist.", return nullptr);

        thread_info_s* current_thread = threads;
        do {

            if(current_thread->thread_id == id) {
                pthread_mutex_unlock(Engine_get_thread_mutex());
                return current_thread;
            }
            current_thread = current_thread->next;
        } while(current_thread);
    });
#elif CENGINE_WIN32
    mutex_locked_code(Engine_get_thread_mutex(), {
        assert(threads != nullptr, "Threads exist.");

        thread_info_s* current_thread = threads;
        do {

            if(current_thread->thread_id.p == id.p) {
                pthread_mutex_unlock(Engine_get_thread_mutex());
                return current_thread;
            }
            current_thread = current_thread->next;
        } while(current_thread);
    });
#endif
    return nullptr;
}

void Engine_start_thread() {
    thread_info_s* info = nullptr;
    while(!info) {
        info = Engine_get_thread_info(pthread_self());
    }
    info->status = THREAD_RUNNING;
}

void Engine_end_thread() {
    thread_info_s* info = Engine_get_thread_info(pthread_self());

    SDL_Event thread_stopped_event;
    SDL_zero(thread_stopped_event);
    thread_stopped_event.type = SDL_EVENT_USER;
    thread_stopped_event.user.data1 = nullptr;
    thread_stopped_event.user.data2 = nullptr;

    if(info->status == THREAD_STOPPING) {
        info->status = THREAD_STOPPED;
        thread_stopped_event.user.code = USER_EVENT_THREAD_STOPPED;
        SDL_PushEvent(&thread_stopped_event);
        return;
    }

    thread_stopped_event.user.code = USER_EVENT_THREAD_STOPPING;
    SDL_PushEvent(&thread_stopped_event);
    info->status = THREAD_STOPPING;
}

pthread_mutex_t* Engine_get_thread_mutex() {
    if(!thread_mutex_initialized) {
        pthread_mutexattr_init(&thread_mutex_attr);
        pthread_mutexattr_settype(&thread_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&thread_mutex, &thread_mutex_attr);
        thread_mutex_initialized = true;
    }
    return &thread_mutex;
}

pthread_mutex_t* Engine_get_actor_mutex() {
    if(!actor_mutex_initialized) {
        return nullptr;
    }
    return &actor_list_mutex;
}

pthread_t Engine_spawn_thread(void * (*routine)(void *), void* arg) {
#if defined(__linux__)
    pthread_t tid = -1;
#elif defined(__WIN32) || defined(_WIN32_WINNT)
    pthread_t tid = {.p = nullptr};
#endif

    mutex_locked_code(Engine_get_thread_mutex(), {
        pthread_create(&tid, NULL, routine, arg);
        if(!threads) {
            threads = malloc(sizeof(thread_info_s));
            threads->status = THREAD_STARTING;
            threads->thread_id = tid;
            threads->next = nullptr;
            threads->prev = nullptr;
        } else {
            thread_info_s* new_thread = malloc(sizeof(thread_info_s));
            new_thread->status = THREAD_STARTING;
            new_thread->thread_id = tid;
            new_thread->next = threads;
            new_thread->prev = nullptr;
            threads = new_thread;
            new_thread->next->prev = new_thread;
        }
    });
#if defined(__linux__)
    if(tid == -1) {
#elif defined(__WIN32) || defined(_WIN32_WINNT)
    if(!tid.p) {
#endif
        Log_printf(LOG_LEVEL_ERROR, "Failed to acquire a new thread, this may not be good.");
    }
    return tid;
}

int Engine_set_real_tickrate(int tickrate_pref) {
    if(tickrate_pref < 0) tickrate_pref = TICK_RATE;
    effective_tick_rate = tickrate_pref;
    return effective_tick_rate;
}

void Engine_get_memory(int* currRealMem, int* peakRealMem, int* currVirtMem, int* peakVirtMem) {
    // stores each word in status file
    char buffer[1024] = "";

    // linux file contains this-process info
    FILE* file = fopen("/proc/self/status", "r");

    // read the entire file
    while (fscanf(file, " %1023s", buffer) == 1) {
        if (strcmp(buffer, "VmRSS:") == 0 && currRealMem) {
            fscanf(file, " %d", currRealMem);
        }
        if (strcmp(buffer, "VmHWM:") == 0 && peakRealMem) {
            fscanf(file, " %d", peakRealMem);
        }
        if (strcmp(buffer, "VmSize:") == 0 && currVirtMem) {
            fscanf(file, " %d", currVirtMem);
        }
        if (strcmp(buffer, "VmPeak:") == 0 && peakVirtMem) {
            fscanf(file, " %d", peakVirtMem);
        }
    }
    fclose(file);
}