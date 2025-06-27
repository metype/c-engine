#ifndef CENGINE_APP_STATE_H
#define CENGINE_APP_STATE_H

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_render.h"

#include "definitions.h"

typedef struct scene scene;

typedef struct perf_metrics {
    long iterate_last_called;
    float dt;
    double time_running;
    float fps;
    int display_dt;
    int max_fps;
    float time_in_tick;
    float tick_timer;
    int tick_rate;
} perf_metrics_s;

typedef struct app_state {
    SDL_Window* window_ptr;
    SDL_Renderer* renderer_ptr;
    SDL_GPUDevice* gpu_device_ptr;
    perf_metrics_s* perf_metrics_ptr;
    volatile scene* scene;
} app_state_s;
#endif //CENGINE_APP_STATE_H
