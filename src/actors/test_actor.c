#include <malloc.h>
#include <stdlib.h>
#include "test_actor.h"
#include "../log.h"
#include "../util.h"
#include "../audio.h"
#include "../math.h"

#define TICK_STR_LEN 16
#define TICK_S_STR_LEN 16
#define REAL_S_STR_LEN 16
#define FPS_STR_LEN 6

void debug_actor_init(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_INIT(self, debug_actor_data_s);

    Log_printf(LOG_LEVEL_DEBUG, "Debugging actor initialized.");
    Audio_set_channel_volume(0, 20);
    Audio_load("/home/emily/Documents/SecuritySimulator/Assets/Audio/Contracts/outside_night_ambiance.wav", "test");
    Audio_play("test", 0);

    if(!data->tick_str) data->tick_str = malloc(sizeof(char) * TICK_STR_LEN);
    if(!data->tick_s_str) data->tick_s_str = malloc(sizeof(char) * TICK_S_STR_LEN);
    if(!data->real_s_str) data->real_s_str = malloc(sizeof(char) * REAL_S_STR_LEN);
    if(!data->fps_str) data->fps_str = malloc(sizeof(char) * FPS_STR_LEN);

    sprintf(data->tick_str, "Tickcount");
    sprintf(data->tick_s_str, "Tick Second");
    sprintf(data->real_s_str, "Second Count");
    sprintf(data->fps_str, "FPS");

    debug_actor_recalc_bb(self);
}

void debug_actor_think(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_THINK(self, debug_actor_data_s);

    snprintf(data->tick_str, 16, "%i", self->ticks_since_spawn);
    snprintf(data->tick_s_str, 16, "%.2f", Engine_ticks_to_seconds(self->ticks_since_spawn));
    snprintf(data->real_s_str, 16, "%.2f", state_ptr->perf_metrics_ptr->time_running);

//    sprintf(test_value_str, "%lu", self->data->test_value);

    if(state_ptr->perf_metrics_ptr->fps > 9999) {
        sprintf(data->fps_str, ">9999");
    } else {
        sprintf(data->fps_str, "%.0f", state_ptr->perf_metrics_ptr->fps);
    }

    debug_actor_recalc_bb(self);
}

void debug_actor_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, debug_actor_data_s);
    //transform_s tr = Actor_get_transform_lerp(self, state_ptr->perf_metrics_ptr->time_in_tick);

    float xPos = 0;//tr.position.x;
    float yPos = 0;//tr.position.y;

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 8  + yPos, data->fps_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 16 + yPos, data->tick_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 24 + yPos, data->tick_s_str);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8 + xPos, 32 + yPos, data->real_s_str);
//    SDL_RenderDebugText(state_ptr->renderer_ptr, 64, 256, test_value_str);
}

void debug_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event) {}

void debug_actor_recalc_bb(actor_s* self) {
    ACTOR_DATA_PTR(self, debug_actor_data_s);

    if(self->bb) free(self->bb);
    self->bb = malloc(sizeof(rect));

    float width = 0;

    width = Maxf(width, (float)strlen(data->fps_str) * 8.f);
    width = Maxf(width, (float)strlen(data->tick_str) * 8.f);
    width = Maxf(width, (float)strlen(data->tick_s_str) * 8.f);
    width = Maxf(width, (float)strlen(data->real_s_str) * 8.f);

    self->bb->tl = (float2_s) {.x = self->transform.position.x + 8, .y = self->transform.position.y + 8};
    self->bb->br = (float2_s) {.x = self->bb->tl.x + width, .y = self->bb->tl.y + 32.f};
}

string* debug_actor_serialize(void* serialized_obj) {
    return so("nil");
}

void* debug_actor_deserialize(string* str) {
    return nullptr;
}