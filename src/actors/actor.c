#include "actor.h"
#include "../log.h"
#include "../errors.h"
#include "../structures/hashmap.h"
#include "test_actor.h"
#include "tilemap_actor.h"
#include "../util.h"
#include <pthread.h>
#include <malloc.h>

#if CENGINE_WIN32
#include "../win32_stdlib.h"
#endif

void empty_init(actor_s* actor, app_state_s* state_ptr) {}
void empty_think(actor_s* actor, app_state_s* state_ptr) {}
void empty_render(actor_s* actor, app_state_s* state_ptr) {}
void empty_event(actor_s* actor, app_state_s* state_ptr, SDL_Event* event) {}

void Actor_tick(actor_s* actor, struct app_state* state_ptr) { // NOLINT(*-no-recursion)
    if(actor == nullptr) return;

    if(actor->ticks_since_spawn == 0) {
        // Do the check once and then assume from here out, these values do not change
        // This isn't necessarily true, but it should be for compliant Actors so
        // Fewer checks make code run faster :)
        if(!actor->init) actor->init = &empty_init;
        if(!actor->thinker) actor->thinker = &empty_think;
        if(!actor->render) actor->render = &empty_render;

        actor->init(actor, state_ptr);
    }

    actor->thinker(actor, state_ptr);

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_tick(actor->children->array[i], state_ptr);
    }

    actor->ticks_since_spawn++;
}

void Actor_render(actor_s* actor, struct app_state* state_ptr) { // NOLINT(*-no-recursion)
    if(!actor) return;
    if(!actor->visible) return;

    actor->render(actor, state_ptr);

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_render(actor->children->array[i], state_ptr);
    }
}

void Actor_event(actor_s* actor, struct app_state* state_ptr, SDL_Event* event) { // NOLINT(*-no-recursion)
    if(!actor) return;

    actor->event(actor, state_ptr, event);

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_event(actor->children->array[i], state_ptr, event);
    }
}

void Actor_update_transforms(actor_s* actor) { // NOLINT(*-no-recursion)
    if(!actor) return;

    actor->pre_transform = actor->transform;

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_update_transforms(actor->children->array[i]);
    }
}

void Actor_add_child(actor_s* parent, actor_s* child) {
    assert(parent != nullptr, "Parent cannot be null!", return);
    assert(child != nullptr, "Child to add cannot be null!", return);
    list_add(parent->children, child);
    child->parent = parent;
}

void Actor_add_sibling(actor_s* sibling, actor_s* new_sibling) {
    assert(sibling != nullptr, "Sibling cannot be null!", return);
    assert(sibling->parent != nullptr, "Trying to add a sibling to a actor that cannot have siblings (no parent).", return);
    Actor_add_child(sibling->parent, new_sibling);
    new_sibling->parent = sibling->parent;
}

void Actor_remove_child(actor_s* parent, actor_s* child) {
    assert(parent != nullptr, "Parent cannot be null!", return);
    assert(child != nullptr, "Child to remove cannot be null!", return);

    for(register int i = 0; i < parent->children->element_count; i++) {
        if(parent->children->array[i] == child) {
            parent->children->array[i] = nullptr;
            break;
        }
    }
    Actor_cleanup_children(parent);
    child->parent = nullptr;
}

void Actor_cleanup_children(actor_s* actor) {
    assert(actor != nullptr, "Parent cannot be null!", return);
    list_remove(actor->children, nullptr);
}

actor_s* Actor_create_s(transform_s transform, char* actor_id) { return Actor_create(transform, Actor_get_def(actor_id)); }

actor_s* Actor_create(transform_s transform, actor_def_s* actor_definition) {
    actor_s* new_actor = malloc(sizeof(actor_s));

    new_actor->actor_id = strdup(actor_definition->actor_id);
    new_actor->ticks_since_spawn = 0;
    new_actor->visible = true;

    new_actor->parent = nullptr;
    new_actor->children = list_new(sizeof(actor_s*));

    new_actor->thinker = actor_definition->thinker;
    new_actor->init = actor_definition->init;
    new_actor->render = actor_definition->render;
    new_actor->event = actor_definition->event;

    new_actor->transform = transform;
    return new_actor;
}

void Actor_destroy(actor_s* actor) {
    free(actor->actor_id);
    free(actor->children->array);
    free(actor->children);
}

hash_map_s* actor_defs = nullptr;
pthread_mutex_t actor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t actor_mutex_attr;

actor_def_s* Actor_get_def(char* actor_id) {
    actor_def_s *def = nullptr;
    mutex_locked_code(&actor_mutex, {
        if (!actor_defs) return nullptr;
        def = map_get(actor_defs, actor_id, actor_def_s*);
    });
    return def;
}

void Actor_register_def(char* actor_id, actor_init(actor_init), actor_thinker(actor_think), actor_render(actor_render), actor_event(actor_event)) {
    if(!actor_id) return;

    if(!actor_init) actor_init = &empty_init;
    if(!actor_render) actor_render = &empty_render;
    if(!actor_think) actor_think = &empty_think;
    if(!actor_event) actor_event = &empty_event;

    actor_def_s* actor_def = malloc(sizeof(actor_def_s));
    actor_def->thinker = actor_think;
    actor_def->init = actor_init;
    actor_def->render = actor_render;
    actor_def->event = actor_event;
    actor_def->actor_id = strdup(actor_id);

    mutex_locked_code(&actor_mutex, {
        if (!actor_defs) {
            actor_defs = malloc(sizeof(hash_map_s));
            Map_init(actor_defs);
        }

        map_set(actor_defs, actor_id, actor_def);
    });

}

void Actor_register_default_defs() {
    pthread_mutex_init(&actor_mutex, &actor_mutex_attr);

    Actor_register_def("test_actor", &debug_actor_init, &debug_actor_think, &debug_actor_render, &debug_actor_event);
    Actor_register_def("tilemap", &tilemap_actor_init, &tilemap_actor_think, &tilemap_actor_render, &tilemap_actor_event);
}

transform_s Actor_get_transform(actor_s* actor) {
    float xPos = actor->transform.position.x;
    float yPos = actor->transform.position.y;

    float xScl = actor->transform.scale.x;
    float yScl = actor->transform.scale.y;

    float rot = actor->transform.rotation;

    actor_s *parent = actor->parent;

    while (parent) {
        xPos += parent->transform.position.x;
        yPos += parent->transform.position.y;

        xScl *= parent->transform.scale.x;
        yScl *= parent->transform.scale.y;

        rot += actor->transform.rotation;

        parent = parent->parent;
    }

    return (transform_s){.rotation = rot, .scale = (float2_s){.x = xScl, .y = yScl}, .position = (float2_s){.x = xPos, .y = yPos}};
}

transform_s Actor_get_transform_lerp(actor_s* actor, float time_in_tick) {
    transform_s local_lerped_tr = Actor_get_local_transform_lerp(actor, time_in_tick);

    float xPos = local_lerped_tr.position.x;
    float yPos = local_lerped_tr.position.y;

    float xScl = local_lerped_tr.scale.x;
    float yScl = local_lerped_tr.scale.y;

    float rot = local_lerped_tr.rotation;

    actor_s *parent = actor->parent;

    while (parent) {
        transform_s parent_lerped_tr = Actor_get_local_transform_lerp(parent, time_in_tick);
        xPos += parent_lerped_tr.position.x;
        yPos += parent_lerped_tr.position.y;

        xScl *= parent_lerped_tr.scale.x;
        yScl *= parent_lerped_tr.scale.y;

        rot += parent_lerped_tr.rotation;

        parent = parent->parent;
    }

    return (transform_s){.rotation = rot, .scale = (float2_s){.x = xScl, .y = yScl}, .position = (float2_s){.x = xPos, .y = yPos}};
}

transform_s Actor_get_local_transform_lerp(actor_s* actor, float time_in_tick) {
    float PxPos = actor->pre_transform.position.x;
    float PyPos = actor->pre_transform.position.y;

    float PxScl = actor->pre_transform.scale.x;
    float PyScl = actor->pre_transform.scale.y;

    float Prot = actor->pre_transform.rotation;

    float xPos = actor->transform.position.x;
    float yPos = actor->transform.position.y;

    float xScl = actor->transform.scale.x;
    float yScl = actor->transform.scale.y;

    float rot = actor->transform.rotation;

    return (transform_s) {
        .rotation = Lerp(Prot, rot, time_in_tick),
        .scale = (float2_s){.x = Lerp(PxScl, xScl, time_in_tick), .y = Lerp(PyScl, yScl, time_in_tick)},
        .position = (float2_s){.x = Lerp(PxPos, xPos, time_in_tick), .y = Lerp(PyPos, yPos, time_in_tick)}
    };
}