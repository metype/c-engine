#ifndef TEST_ACTOR_H
#define TEST_ACTOR_H
#include "actor.h"

typedef struct debug_actor_data_s {
    char *tick_str;
    char *tick_s_str;
    char *real_s_str;
    char *fps_str;
} debug_actor_data_s;

void debug_actor_init(actor_s* self, app_state_s* state_ptr);
void debug_actor_think(actor_s* self, app_state_s* state_ptr);
void debug_actor_render(actor_s* self, app_state_s* state_ptr);
void debug_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event);
void debug_actor_recalc_bb(actor_s* self);

string* debug_actor_serialize(void* serialized_obj);
void* debug_actor_deserialize(string* str);
#endif //TEST_ACTOR_H