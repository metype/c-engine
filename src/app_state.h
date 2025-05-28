#ifndef CENGINE_APP_STATE_H
#define CENGINE_APP_STATE_H

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_render.h"

typedef struct perf_metrics {
    long iterate_last_called;
    float dt;
    double time_running;
    float fps;
    int display_dt;
    int max_fps;
    float time_in_tick;
} perf_metrics;

typedef struct app_state {
    SDL_Window* window_ptr;
    SDL_Renderer* renderer_ptr;
    SDL_GPUDevice* gpu_device_ptr;
    perf_metrics* perf_metrics_ptr;
} app_state;
#endif //CENGINE_APP_STATE_H
