#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

#include <stdlib.h>
#include "engine.h"
#include "audio.h"
#include "app_state.h"
#include "actors/actor.h"
#include "log.h"
#include "rendering.h"
#include "filesystem.h"
#include "string.h"

#include "tests/test_suite.h"
#include "tests/serialization_tests.h"

#include "arg.h"

#if CENGINE_LINUX
#include "getopt.h"
#include "util.h"
#include <sys/time.h>
#include <SDL3_image/SDL_image.h>
#elif CENGINE_WIN32

#include "timeapi.h"
#include "config.h"
#include "win32_stdlib.h"
#include "win32_getopt.h"
#include "util.h"

#endif

void register_and_run_tests();
void load_args(int argc, char** argv, argument * all_args, int list_len);
void draw_splash(app_state_s* state_ptr);

SDL_AppResult SDL_AppInit(void **application_state, int argc, char **argv) {
    struct timeval tp1;
    gettimeofday(&tp1, nullptr);
    long us1 = tp1.tv_sec * 1000000 + tp1.tv_usec;

    FS_init();
    Log_init();

    app_state_s* state_ptr = malloc(sizeof(app_state_s));
    state_ptr->perf_metrics_ptr = malloc(sizeof(perf_metrics_s));
    state_ptr->perf_metrics_ptr->fps_arr_len = 32;
    state_ptr->perf_metrics_ptr->fps_arr_idx = 0;
    state_ptr->perf_metrics_ptr->previous_fps_arr = malloc(sizeof(float) * state_ptr->perf_metrics_ptr->fps_arr_len);

    for(int i = 0; i < state_ptr->perf_metrics_ptr->fps_arr_len; i++) {
        state_ptr->perf_metrics_ptr->previous_fps_arr[i] = 1.f;
    }

    struct argument* arg_map = malloc(sizeof(struct argument));
    int arg_idx = 0;
    int arg_list_len = 1;

    int do_tests = 0;
    int no_splash = 0;
    state_ptr->perf_metrics_ptr->tick_rate = -1;

    ARG_DEF(arg_map, arg_idx, arg_list_len, "test", 't', false, &do_tests);
    ARG_DEF(arg_map, arg_idx, arg_list_len, "tickrate", 'r', true, &state_ptr->perf_metrics_ptr->tick_rate);
    ARG_DEF(arg_map, arg_idx, arg_list_len, "nosplash", 0, false, &no_splash);

    load_args(argc, argv, arg_map, arg_idx);

    free(arg_map);

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        Log_printf(LOG_LEVEL_FATAL, "Could not init video: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("CEngine", 1920, 1080, SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        Log_printf(LOG_LEVEL_FATAL, "Could not create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "");
    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

    state_ptr->window_ptr = window;
    state_ptr->renderer_ptr = renderer;
    state_ptr->perf_metrics_ptr->dt = 0;
    state_ptr->perf_metrics_ptr->time_running = 0;
    state_ptr->scene = R_init_scene(state_ptr->renderer_ptr);

    struct timeval tp;
    gettimeofday(&tp, nullptr);
    state_ptr->perf_metrics_ptr->iterate_last_called = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    Actor_register_default_defs();
    Audio_init();
    Engine_spawn_thread(&Engine_tick, state_ptr);

    if(!no_splash) draw_splash(state_ptr);

    SDL_SetHint( SDL_HINT_FRAMEBUFFER_ACCELERATION, "1" );

    *application_state = state_ptr;

    if(do_tests) register_and_run_tests();

    struct timeval tp2;
    gettimeofday(&tp2, nullptr);
    long us2 = tp2.tv_sec * 1000000 + tp2.tv_usec;

    float delta = (float)(us2 - us1) * 0.000001f;

    if(!no_splash) {
        Engine_pause_all_threads(); // Submit a kind request to every thread that they maybe chill
        Thread_sleep(3.f - delta); // Try and make sure we wait about 3 seconds after program start
        Engine_unpause_all_threads(); // Okay they can go right ahead and start running again :)
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return SDL_APP_CONTINUE;
}

void draw_splash(app_state_s* state_ptr) {
    SDL_Renderer* renderer = state_ptr->renderer_ptr;

    char* path_str = malloc(sizeof(char) * FS_PATH_MAX);
    sprintf(path_str, "%s/splash.png", FS_get_working_dir());

    SDL_Texture* splash = IMG_LoadTexture(renderer, path_str);
    SDL_RenderTexture(renderer, splash, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    //cleanup
    free(path_str);
    SDL_DestroyTexture(splash);
}

void ref_count_leak_test() {
    string* str_1 = s("Hello, world!");

    s_cat(str_1, so(" And a goodbye too!"));
    Log_print(LOG_LEVEL_DEBUG, str_1->c_str);

    s_rep(str_1, so("!"), so("?"));
    Log_print(LOG_LEVEL_DEBUG, str_1->c_str);

    s_rep_n(str_1, so("?"), so("!"), 1);
    Log_print(LOG_LEVEL_DEBUG, str_1->c_str);

    ref_dec(&str_1->refcount);
}

void register_and_run_tests() {
    Engine_pause_all_threads();

    T_register_test((test){&serialization_test_run, "Serialization Test"});
    T_register_test((test){&deserialization_test_run, "Deserialization Test"});
    T_run_all_tests();

    Engine_unpause_all_threads();
}

SDL_AppResult SDL_AppIterate(void *application_state) {
    int current_virtual_memory_footprint;
    Engine_get_memory(nullptr, nullptr, &current_virtual_memory_footprint, nullptr);

    // if we've allocated over 8 gigs, something's wrong
    // I'd rather crash than take the system down with a mem leak
    if(current_virtual_memory_footprint > 8 * 1024 * 1024) {
        Log_printf(LOG_LEVEL_ERROR, "Memory footprint (%u kB) exceeds 8 GB engine limit! Aborting!", current_virtual_memory_footprint);
        return SDL_APP_FAILURE;
    }

    if(application_state == nullptr) return SDL_APP_FAILURE;
    app_state_s* state_ptr = (app_state_s*)(application_state);

    struct timeval tp;
    gettimeofday(&tp, nullptr);
    long us = tp.tv_sec * 1000000 + tp.tv_usec;

    long delta = us - state_ptr->perf_metrics_ptr->iterate_last_called;
    state_ptr->perf_metrics_ptr->iterate_last_called = us;
    state_ptr->perf_metrics_ptr->dt = (float)delta * 0.000001f;
    if(state_ptr->perf_metrics_ptr->dt > 1) {
        return SDL_APP_CONTINUE;
    }
    state_ptr->perf_metrics_ptr->time_running += state_ptr->perf_metrics_ptr->dt;
    state_ptr->perf_metrics_ptr->previous_fps_arr[state_ptr->perf_metrics_ptr->fps_arr_idx++] = (1.0f / state_ptr->perf_metrics_ptr->dt);
    state_ptr->perf_metrics_ptr->fps_arr_idx %= state_ptr->perf_metrics_ptr->fps_arr_len;

    state_ptr->perf_metrics_ptr->fps = 0.f;
    for(register int i = 0; i < state_ptr->perf_metrics_ptr->fps_arr_len; i++) {
        state_ptr->perf_metrics_ptr->fps += state_ptr->perf_metrics_ptr->previous_fps_arr[i];
    }
    state_ptr->perf_metrics_ptr->fps /= (float) state_ptr->perf_metrics_ptr->fps_arr_len;

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state_ptr->renderer_ptr);

    int err = pthread_mutex_trylock(Engine_get_actor_mutex());
    while(err == EBUSY) {
        err = pthread_mutex_trylock(Engine_get_actor_mutex());
    }

    if(err == 0) {
        state_ptr->perf_metrics_ptr->time_in_tick = Minf(
                state_ptr->perf_metrics_ptr->tick_timer / Engine_get_tick_frate(), 1);
        state_ptr->perf_metrics_ptr->tick_timer += state_ptr->perf_metrics_ptr->dt;
        R_render_scene(state_ptr, state_ptr->scene);
        pthread_mutex_unlock(Engine_get_actor_mutex());
    }


    SDL_RenderPresent(state_ptr->renderer_ptr);

    thread_info_s* threads = Engine_get_threads();
    thread_info_s* cur_info = threads;

    // Loop through every thread, if they're all stopped, we return SDL_APP_SUCCESS and quit, otherwise we return SDL_APP_CONTINUE and iterate again.
    do {
        switch (cur_info->status) {
            case THREAD_STOPPED: // If this thread is stopped, do nothing and break, moving to the next thread
                break;
            case THREAD_STOPPING: // If this thread is stopping, stop it. The THREAD_STOPPING state means the thread is ready to die, and waiting on someone to kill it
                cur_info->status = THREAD_STOPPED;
            default: // Intentionally fall through from the THREAD_STOPPING case here, and in all unhandled cases, return SDL_APP_CONTINUE
                return SDL_APP_CONTINUE;
        }
        cur_info = cur_info->next;
    } while (cur_info);

    return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void *application_state, SDL_Event *event) {
    app_state_s* state_ptr = (app_state_s*)(application_state);

    thread_info_s* threads = Engine_get_threads();
    thread_info_s* cur_info = threads;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            do {
                cur_info->status = THREAD_STOP_REQUESTED;
                cur_info = cur_info->next;
            } while (cur_info);
            break;
    }
    Engine_event(state_ptr, event);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *application_state, SDL_AppResult result) {
    if(application_state == nullptr) return;
    app_state_s* state_ptr = (app_state_s*)(application_state);
    SDL_DestroyWindow(state_ptr->window_ptr);
    Audio_free();
    Log_quit();
    SDL_Quit();
}

void load_args(int argc, char** argv, argument* all_args, int list_len) {
    while(true) {
        struct option* long_opts = malloc(sizeof(struct option) * list_len);
        string* short_opts_str_str = s("");
        int it = 0;
        for(int i = 0; i < list_len; i++) {
            s_cat(short_opts_str_str, S_final(S_char_to_str(all_args[i].short_opt)));
            s_cat(short_opts_str_str, so(all_args[i].required ? ":" : ""));
            long_opts[it++] = (struct option){all_args[i].long_opt, (all_args[i].required ? required_argument : no_argument), nullptr, all_args[i].short_opt};
        }
        const char *short_opts = short_opts_str_str->c_str;

        int option_index = 0;

        int c = getopt_long(argc, argv, short_opts, long_opts, &option_index);

        free(long_opts);
        ref_dec(&short_opts_str_str->refcount);

        if (c == -1) {

            break;
        }

        for(int i = 0; i < list_len; i++) {
            if(c == all_args[i].short_opt) {
                *all_args[i].val = all_args[i].required ? (int)strtol(optarg, nullptr, 10) : 1;
            }
        }
    }
}
