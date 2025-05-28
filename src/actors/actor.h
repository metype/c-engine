#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H

#include "../app_state.h"
#include "../callbacks.h"
#include "../serialization.h"

#define actor_thinker(name) CALLBACK(name, void, struct actor*)
#define actor_init(name) CALLBACK(name, void, struct actor*)
#define actor_render(name) CALLBACK(name, void, struct actor*, struct app_state*)

typedef struct actor {
    float x, y;
    int ticks_since_spawn;
    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);
    struct actor* next;
} actor;

typedef struct actor_def {
    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);
} actor_def;

void A_tick(actor* root);
actor* A_get_end_ptr(actor* root);
void A_spawn(actor* root, actor* new_actor);
void A_despawn(actor* root, actor* to_despawn);
actor* A_make_actor(float x, float y, actor_def* actor_definition);
actor_def* A_get_actor_def(char* actor_id);
void A_register_actor_def(char* actor_id, actor_def* actor_def);
void A_register_default_actors();

#endif //CPROJ_ACTOR_H
