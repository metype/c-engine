#ifndef TEST_ACTOR_H
#define TEST_ACTOR_H
#include "actor.h"

void debug_actor_init(actor* self);
void debug_actor_think(actor* self);
void debug_actor_render(actor* self, app_state* state_ptr);

#endif //TEST_ACTOR_H