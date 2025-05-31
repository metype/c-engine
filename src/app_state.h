#ifndef CENGINE_APP_STATE_H
#define CENGINE_APP_STATE_H

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_render.h"

typedef struct perf_metrics {
    long iterate_last_called;
    float dt;
    double time_running;
    float fps;
    __attribute__((unused)) int display_dt;
    __attribute__((unused)) int max_fps;
    __attribute__((unused)) float time_in_tick;
    int tick_rate;
} perf_metrics_s;

typedef struct app_state {
    SDL_Window* window_ptr;
    SDL_Renderer* renderer_ptr;
    __attribute__((unused)) SDL_GPUDevice* gpu_device_ptr;
    perf_metrics_s* perf_metrics_ptr;
} app_state_s;
#endif //CENGINE_APP_STATE_H
