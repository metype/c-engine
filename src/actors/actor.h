#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H

#include "../app_state.h"
#include "../callbacks.h"
#include "../serialization.h"

#define actor_thinker(name) CALLBACK(name, void, struct actor_s*)
#define actor_init(name) CALLBACK(name, void, struct actor_s*)
#define actor_render(name) CALLBACK(name, void, struct actor_s*, struct app_state*)

typedef struct actor_s {
    float x, y;
    int ticks_since_spawn;
    char* actor_id;

    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);

    struct actor_s* next;
    struct actor_s* prev;
} actor_s;

typedef struct actor_def_s {
    char* actor_id;

    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);
} actor_def_s;

void Actor_tick(actor_s* actor);
actor_s* Actor_list_end(actor_s* root);
void Actor_spawn(actor_s* root, actor_s* new_actor);
void Actor_despawn(actor_s* to_despawn);
actor_s* Actor_create(float x, float y, actor_def_s* actor_definition);
actor_def_s* Actor_get_def(char* actor_id);
void Actor_register_def(char* actor_id, actor_init(actor_init), actor_thinker(actor_think), actor_render(actor_render));
void Actor_register_default_defs();

#endif //CPROJ_ACTOR_H
