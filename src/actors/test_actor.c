#include <malloc.h>
#include "test_actor.h"
#include "../log.h"
#include "../util.h"

void debug_actor_init(actor* self) {
    L_printf(LOG_LEVEL_DEBUG, "Debugging actor initialized.");
}

void debug_actor_think(actor* self) {

}

void debug_actor_render(actor* self, app_state* state_ptr) {
    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 255, 255, 255, SDL_ALPHA_OPAQUE);

    char* tick_str = malloc(sizeof(char) * 48);
    sprintf(tick_str, "%i", self->ticks_since_spawn);

    char* tick_s_str = malloc(sizeof(char) * 48);
    sprintf(tick_s_str, "%.2f", E_ticks_to_seconds(self->ticks_since_spawn));

    char* fps_str = malloc(sizeof(char) * 6);

    if(state_ptr->perf_metrics_ptr->fps > 9999) {
        sprintf(fps_str, ">9999");
    } else {
        sprintf(fps_str, "%.0f", state_ptr->perf_metrics_ptr->fps);
    }

    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 8, fps_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 16, tick_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 24, tick_s_str);


    free(tick_str);
    free(tick_s_str);
    free(fps_str);
}