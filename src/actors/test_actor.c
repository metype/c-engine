#include <malloc.h>
#include <stdlib.h>
#include "test_actor.h"
#include "../log.h"
#include "../util.h"
#include "../audio.h"

char tick_str[16];
char tick_s_str[16];
char real_s_str[16];
char fps_str[6];

char test_value_str[32];

void debug_actor_init(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_INIT(self, debug_actor_data_s);

    Log_printf(LOG_LEVEL_DEBUG, "Debugging actor initialized.");
    Audio_set_channel_volume(0, 20);
    Audio_load("/home/emily/Documents/SecuritySimulator/Assets/Audio/Contracts/outside_night_ambiance.wav", "test");
    Audio_play("test", 0);
}

void debug_actor_think(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_THINK(self, debug_actor_data_s);

    snprintf(tick_str, 16, "%i", self->ticks_since_spawn);
    snprintf(tick_s_str, 16, "%.2f", Engine_ticks_to_seconds(self->ticks_since_spawn));
    snprintf(real_s_str, 16, "%.2f", state_ptr->perf_metrics_ptr->time_running);

//    sprintf(test_value_str, "%lu", self->data->test_value);

    if(state_ptr->perf_metrics_ptr->fps > 9999) {
        sprintf(fps_str, ">9999");
    } else {
        sprintf(fps_str, "%.0f", state_ptr->perf_metrics_ptr->fps);
    }
//
//    float mx;
//    float my;
//
//    SDL_GetMouseState(&mx, &my);
//
//    self->transform.position = (float2_s){.x = mx, .y = my};
}

void debug_actor_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, debug_actor_data_s);
    //transform_s tr = Actor_get_transform_lerp(self, state_ptr->perf_metrics_ptr->time_in_tick);

    float xPos = 0;//tr.position.x;
    float yPos = 0;//tr.position.y;

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 8  + yPos, fps_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 16 + yPos, tick_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 24 + yPos, tick_s_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 32 + yPos, real_s_str);
//    SDL_RenderDebugText(state_ptr->renderer_ptr, 64, 256, test_value_str);
}

void debug_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event) {}