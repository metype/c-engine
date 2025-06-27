#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H

#include "../app_state.h"
#include "../callbacks.h"
#include "../serialization.h"
#include "../structures/list.h"
#include "../transform.h"

#include <SDL3/SDL_events.h>

#define ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type) custom_actor_data_type* data = actor_ptr->data

#define ACTOR_PRE_INIT(actor_ptr, custom_actor_data_type) actor_ptr->data = malloc(sizeof(custom_actor_data_type)); ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)
#define ACTOR_PRE_THINK(actor_ptr, custom_actor_data_type) ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)
#define ACTOR_PRE_RENDER(actor_ptr, custom_actor_data_type) ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)
#define ACTOR_PRE_EVENT(actor_ptr, custom_actor_data_type) ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)

#define actor_thinker(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*)
#define actor_init(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*)
#define actor_render(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*)
#define actor_event(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*, SDL_Event*)

typedef struct actor_s {
    transform_s transform;
    transform_s pre_transform;

    bool visible;
    int ticks_since_spawn;
    char* actor_id;

    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);
    actor_event(event);

    struct actor_s* parent;
    list_s* children;

    void* data;
} actor_s;

typedef struct actor_def_s {
    char* actor_id;

    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);
    actor_event(event);
} actor_def_s;

void Actor_tick(actor_s* actor, struct app_state* state_ptr);
void Actor_render(actor_s* actor, struct app_state* state_ptr);
void Actor_event(actor_s* actor, struct app_state* state_ptr, SDL_Event* event);
void Actor_update_transforms(actor_s* actor);

void Actor_add_child(actor_s* parent, actor_s* child);
void Actor_add_sibling(actor_s* sibling, actor_s* new_sibling);
void Actor_remove_child(actor_s* parent, actor_s* child);
void Actor_cleanup_children(actor_s* actor);

actor_s* Actor_create_s(transform_s transform, char* actor_id);
actor_s* Actor_create(transform_s transform, actor_def_s* actor_definition);
actor_def_s* Actor_get_def(char* actor_id);

/**
 *  Destroys an actor, frees all memory associated with it and removes the actor from
 *  the child list of its parent if applicable.
 *
 *  Notably, this method does NOT free or delete any children, but does free the
 *  children array, so be aware of that.
 *
 * \param actor The actor to destroy
 *
 * \returns void
 *
 * \threadsafety This function is not thread safe, make sure you're the only one who
 *  can access the actor pointer when calling this function.
 */
void Actor_destroy(actor_s* actor);

void Actor_register_def(char* actor_id, actor_init(actor_init), actor_thinker(actor_think), actor_render(actor_render), actor_event(actor_event));
void Actor_register_default_defs();

/**
 *  Recalculates an actor's transformation, propagates parent transformations.
 *  Internal method, really not designed to be called except from in internal code
 *  that modifies transforms.
 *
 * \param actor The actor to recalculate the transform of
 *
 * \returns void
 *
 * \threadsafety This function is not thread safe, make sure you're the only one who
 *  can access the actor pointer when calling this function.
 */
transform_s Actor_get_transform(actor_s* actor);
transform_s Actor_get_transform_lerp(actor_s* actor, float time_in_tick);
transform_s Actor_get_local_transform_lerp(actor_s* actor, float time_in_tick);

#endif //CPROJ_ACTOR_H
