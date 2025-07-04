#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H

#include "../app_state.h"
#include "../callbacks.h"
#include "../serialization.h"
#include "../structures/list.h"
#include "../transform.h"

#include <SDL3/SDL_events.h>

#define ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type) custom_actor_data_type* data = actor_ptr->data; if(!data) return

#define ACTOR_PRE_INIT(actor_ptr, custom_actor_data_type) if(!actor_ptr->data) { actor_ptr->data = malloc(sizeof(custom_actor_data_type)); } ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)
#define ACTOR_PRE_THINK(actor_ptr, custom_actor_data_type) ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)
#define ACTOR_PRE_RENDER(actor_ptr, custom_actor_data_type) ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)
#define ACTOR_PRE_EVENT(actor_ptr, custom_actor_data_type) ACTOR_DATA_PTR(actor_ptr, custom_actor_data_type)

#define actor_thinker(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*)
#define actor_init(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*)
#define actor_render(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*)
#define actor_event(name) M_CALLBACK(name, void, struct actor_s*, struct app_state*, SDL_Event*)

typedef struct script_s script;

typedef struct actor_s {
    transform_s transform; // Holds position, rotation, and scale data of this actor.
    transform_s pre_transform; // Holds position, rotation, and scale data of this actor on the previous tick, used for lerping in render.

    bool visible; // If true, render is called on this actor and its children. If not, rendering is skipped for this actor and its children.
    int ticks_since_spawn; // Set to 0 at actor creation and incremented once every time its thinker is called.
    char* actor_id; // The ID of this actor, non-unique.
    char* name; // The name of this actor, should be unique in parent.
    int handle; // The unique handle for this actor, only valid while the actor is valid.

    actor_thinker(thinker); // Thinker method.
    actor_init(init); // Initialization method.
    actor_render(render); // Rendering method.
    actor_render(late_render); // Late rendering method, called after all children are processed.
    actor_event(event); // Event handling method.

    struct actor_s* parent; // Pointer to the parent actor.
    list_s* children; // List of children actors.

    void* data; // Custom actor data.
    script* script; // Pointer to the script on this actor.
} actor_s;

typedef struct actor_def_s {
    char* actor_id;

    actor_thinker(thinker);
    actor_init(init);
    actor_render(render);
    actor_render(late_render);
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
char* Actor_get_path(actor_s* actor);

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

void Actor_register_def(char* actor_id, actor_init(actor_init), actor_thinker(actor_think), actor_render(actor_render), actor_render(actor_late_render), actor_event(actor_event));
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

actor_s* Actor_from_handle(int handle);

string* Actor_serialize(void* serialized_obj);

void* Actor_deserialize(string* str);

actor_s* Actor_from_path(const char* path);
actor_s* Actor_from_path_rel(actor_s* actor, const char* path);

serialization_func(Actor_get_serialization_func(const char* id));
deserialization_func(Actor_get_deserialization_func(const char* id));

void Actor_register_serialization_func(char* id, serialization_func(serializer));
void Actor_register_deserialization_func(char* id, deserialization_func(deserializer));

#endif //CPROJ_ACTOR_H
