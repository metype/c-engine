#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H

#include "../app_state.h"

#define actor_thinker(name) void (*name)(struct actor*, struct app_state* state_ptr)
#define actor_init(name) void (*name)(struct actor*, struct app_state* state_ptr)

typedef struct actor {
    float x, y;
    int ticks_since_spawn;
    actor_thinker(thinker);
    actor_init(init);
    struct actor* next;
} actor;

typedef struct actor_def {
    actor_thinker(thinker);
    actor_init(init);
} actor_def;

void A_tick(actor* root, app_state* state_ptr);
actor* A_get_end_ptr(actor* root);
void A_spawn(actor* root, actor* new_actor);
void A_despawn(actor* root, actor* to_despawn);
actor* A_make_actor(float x, float y, actor_def* actor_definition);
actor_def* A_get_actor_def(char* actor_id);
void A_register_actor_def(char* actor_id, actor_def* actor_def);
void A_register_default_actors();

#endif //CPROJ_ACTOR_H
