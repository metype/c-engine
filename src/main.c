#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

#include <stdlib.h>
#include <sys/time.h>

#include "engine.h"
#include "audio.h"
#include "app_state.h"
#include "actors/actor.h"
#include "log.h"
#include "rendering.h"
#include "filesystem.h"
#include "string.h"

#include "tests/serialization_tests.h"


SDL_AppResult SDL_AppInit(void **application_state, int argc, char **argv) {
    F_init();
    L_init();

    app_state* state_ptr = malloc(sizeof(app_state));

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        L_printf(LOG_LEVEL_FATAL, "Could not init video: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("Window", 1920, 1080, SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        L_printf(LOG_LEVEL_FATAL, "Could not create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "");
    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

    state_ptr->window_ptr = window;
    state_ptr->renderer_ptr = renderer;
    state_ptr->perf_metrics_ptr = malloc(sizeof(perf_metrics));
    state_ptr->perf_metrics_ptr->dt = 0;

    struct timeval tp;
    gettimeofday(&tp, nullptr);
    state_ptr->perf_metrics_ptr->iterate_last_called = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    A_register_default_actors();
    M_init();
    E_spawn_thread(&E_tick, state_ptr);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, 8, 8, "Initializing");
    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    *application_state = state_ptr;

    serialization_test_run();
    deserialization_test_run();

    return SDL_APP_CONTINUE;
}

void ref_count_leak_test() {
    string* str_1 = s("Hello, world!");

    s_cat(str_1, so(" And a goodbye too!"));
    L_print(LOG_LEVEL_DEBUG, str_1->c_str);

    s_rep(str_1, so("!"), so("?"));
    L_print(LOG_LEVEL_DEBUG, str_1->c_str);

    s_rep_n(str_1, so("?"), so("!"), 1);
    L_print(LOG_LEVEL_DEBUG, str_1->c_str);

    ref_dec(&str_1->refcount);
}

SDL_AppResult SDL_AppIterate(void *application_state) {
    if(application_state == nullptr) return SDL_APP_FAILURE;
    app_state* state_ptr = (app_state*)(application_state);

//    ref_count_leak_test();

    struct timeval tp;
    gettimeofday(&tp, nullptr);
    long us = tp.tv_sec * 1000000 + tp.tv_usec;

    long delta = us - state_ptr->perf_metrics_ptr->iterate_last_called;
    state_ptr->perf_metrics_ptr->iterate_last_called = us;
    state_ptr->perf_metrics_ptr->dt = (float)delta * 0.00001f;
    state_ptr->perf_metrics_ptr->time_running += state_ptr->perf_metrics_ptr->dt;
    state_ptr->perf_metrics_ptr->fps = (1.0f / state_ptr->perf_metrics_ptr->dt);

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state_ptr->renderer_ptr);

    actor* actor_list = E_get_actors();
    actor* current_actor = actor_list;

    do {
        if(current_actor->render) current_actor->render(current_actor, state_ptr);
        current_actor = current_actor->next;
    } while(current_actor);

    E_release_actors();

    SDL_RenderPresent(state_ptr->renderer_ptr);

    thread_info* threads = E_get_threads();
    thread_info* cur_info = threads;

    do {
        switch (cur_info->status) {
            case THREAD_STOPPING:
                cur_info->status = THREAD_STOPPED;
            case THREAD_RUNNING:
            case THREAD_STARTING:
            case THREAD_STOP_REQUESTED:
                return SDL_APP_CONTINUE;
        }
        cur_info = cur_info->next;
    } while (cur_info);

    return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void *application_state, SDL_Event *event) {
//    auto* state_ptr = static_cast<struct app_state*>(app_state);
//    SDL_AppResult res = state_ptr->game_ptr->event(state_ptr, event);
//    if(res != SDL_APP_CONTINUE) return res;
//    gui::event(event);
//    state_ptr->input_manager_ptr->event(state_ptr, event);
    thread_info* threads = E_get_threads();
    thread_info* cur_info = threads;
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

void SDL_AppQuit(void *application_state, SDL_AppResult result) {
    if(application_state == nullptr) return;
    app_state* state_ptr = (app_state*)(application_state);
    SDL_DestroyWindow(state_ptr->window_ptr);
    M_free();
    L_quit();
    SDL_Quit();
}