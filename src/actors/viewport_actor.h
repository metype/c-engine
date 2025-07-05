#ifndef VIEWPORT_ACTOR_H
#define VIEWPORT_ACTOR_H
#include "actor.h"

typedef struct viewport_s viewport;

typedef struct viewport_actor_data_s {
    viewport* vp;
    int x;
    int y;
    int w;
    int h;
} viewport_actor_data_s;

void viewport_actor_init(actor_s* self, app_state_s* state_ptr);
void viewport_actor_think(actor_s* self, app_state_s* state_ptr);
void viewport_actor_render(actor_s* self, app_state_s* state_ptr);
void viewport_actor_late_render(actor_s* self, app_state_s* state_ptr);
void viewport_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event);
void viewport_actor_recalc_bb(actor_s* self);

string* viewport_actor_serialize(void* serialized_obj);
void* viewport_actor_deserialize(string* str);

#endif //VIEWPORT_ACTOR_H