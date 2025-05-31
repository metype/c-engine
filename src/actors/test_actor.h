#ifndef TEST_ACTOR_H
#define TEST_ACTOR_H
#include "actor.h"

void debug_actor_init(__attribute__((unused)) actor_s* self);
void debug_actor_think(__attribute__((unused)) actor_s* self);
void debug_actor_render(actor_s* self, app_state_s* state_ptr);

#endif //TEST_ACTOR_H