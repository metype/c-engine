#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H

#include "../app_state.h"

#define actor_thinker(name) void (*name)(struct actor*, struct app_state* state_ptr)

typedef struct actor {
    float x, y;
    int ticks_since_spawn;
    actor_thinker(thinker);
    struct actor* next;
} actor;

void A_tick(actor* root, app_state* state_ptr);
actor* A_get_end_ptr(actor* root);
void A_spawn(actor* root, actor* new_actor);
void A_despawn(actor* root, actor* to_despawn);
actor* A_make_actor(float x, float y, actor_thinker(thinker));

#endif //CPROJ_ACTOR_H
