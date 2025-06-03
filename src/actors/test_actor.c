#include <malloc.h>
#include "test_actor.h"
#include "../log.h"
#include "../util.h"
#include "../audio.h"

char tick_str[16];
char tick_s_str[16];
char real_s_str[16];
char fps_str[6];

void debug_actor_init(__attribute__((unused)) actor_s* self) {
    Log_printf(LOG_LEVEL_DEBUG, "Debugging actor initialized.");
    Audio_set_channel_volume(0, 20);
    Audio_load("/home/emily/Documents/SecuritySimulator/Assets/Audio/Contracts/outside_night_ambiance.wav", "test");
    Audio_play("test", 0);
}

void debug_actor_think(__attribute__((unused)) actor_s* self) {

}

void debug_actor_render(actor_s* self, app_state_s* state_ptr) {
    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 255, 255, 255, SDL_ALPHA_OPAQUE);

    snprintf(tick_str, 16, "%i", self->ticks_since_spawn);
    snprintf(tick_s_str, 16, "%.2f", Engine_ticks_to_seconds(self->ticks_since_spawn));
    snprintf(real_s_str, 16, "%.2f", state_ptr->perf_metrics_ptr->time_running);

    if(state_ptr->perf_metrics_ptr->fps > 9999) {
        sprintf(fps_str, ">9999");
    } else {
        sprintf(fps_str, "%.0f", state_ptr->perf_metrics_ptr->fps);
    }

    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 8, fps_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 16, tick_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 24, tick_s_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 32, real_s_str);
}