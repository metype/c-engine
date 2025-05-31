#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

#include <sys/time.h>
#include <stdlib.h>

#include "getopt.h"
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

void register_and_run_tests();
void load_args(int argc, char** argv, argument * all_args, int list_len);

SDL_AppResult SDL_AppInit(void **application_state, int argc, char **argv) {
    FS_init();
    Log_init();

    app_state_s* state_ptr = malloc(sizeof(app_state_s));
    state_ptr->perf_metrics_ptr = malloc(sizeof(perf_metrics_s));

    struct argument* arg_map = malloc(sizeof(struct argument));
    int arg_idx = 0;
    int arg_list_len = 1;

    int do_tests = 0;
    state_ptr->perf_metrics_ptr->tick_rate = -1;

    ARG_DEF(arg_map, arg_idx, arg_list_len, "test", 't', false, &do_tests);
    ARG_DEF(arg_map, arg_idx, arg_list_len, "tickrate", 'r', true, &state_ptr->perf_metrics_ptr->tick_rate);

    load_args(argc, argv, arg_map, arg_idx);

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        Log_printf(LOG_LEVEL_FATAL, "Could not init video: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("Window", 1920, 1080, SDL_WINDOW_RESIZABLE);

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

    struct timeval tp;
    gettimeofday(&tp, nullptr);
    state_ptr->perf_metrics_ptr->iterate_last_called = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    Actor_register_default_defs();
    Audio_init();
    Engine_spawn_thread(&Engine_tick, state_ptr);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, 8, 8, "Initializing");
    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    *application_state = state_ptr;

    if(do_tests) register_and_run_tests();

    return SDL_APP_CONTINUE;
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
    state_ptr->perf_metrics_ptr->fps = (1.0f / state_ptr->perf_metrics_ptr->dt);

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state_ptr->renderer_ptr);

    R_render_scene(state_ptr->renderer_ptr);

    actor_s* actor_list = Engine_get_actors();
    mutex_locked_code(Engine_get_actor_mutex(), {
        actor_s* current_actor = actor_list;

        do {
            if(current_actor->render) current_actor->render(current_actor, state_ptr);
            current_actor = current_actor->next;
        } while(current_actor);
    });

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

SDL_AppResult SDL_AppEvent(__attribute__((unused)) void *application_state, SDL_Event *event) {
//    auto* state_ptr = static_cast<struct app_state_s*>(app_state_s);
//    SDL_AppResult res = state_ptr->game_ptr->event(state_ptr, event);
//    if(res != SDL_APP_CONTINUE) return res;
//    gui::event(event);
//    state_ptr->input_manager_ptr->event(state_ptr, event);
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
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *application_state, __attribute__((unused)) SDL_AppResult result) {
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
        const char *const short_opts = short_opts_str_str->c_str;

        int option_index = 0;

        int c = getopt_long(argc, argv, short_opts, long_opts, &option_index);

        if (c == -1)
            break;

        for(int i = 0; i < list_len; i++) {
            if(c == all_args[i].short_opt) {
                *all_args[i].val = all_args[i].required ? (int)strtol(optarg, nullptr, 10) : 1;
            }
        }

        ref_dec(&short_opts_str_str->refcount);
    }
}
