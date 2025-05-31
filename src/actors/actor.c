#include "actor.h"
#include "../log.h"
#include "../errors.h"
#include "../hashmap.h"
#include "test_actor.h"
#include <malloc.h>
#include <pthread.h>

void Actor_tick(actor_s* actor) {
    if(actor == nullptr) return;

    if(actor->init && actor->ticks_since_spawn == 0) actor->init(actor);
    if(actor->thinker) actor->thinker(actor);

    actor->ticks_since_spawn++;
}

struct actor_s* Actor_list_end(actor_s* root) {
    assert(root != nullptr, "Non-null root");
    actor_s* cur_actor = root;
    do {
        cur_actor = cur_actor->next;
    } while(cur_actor->next);
    return cur_actor;
}

void Actor_spawn(actor_s* root, actor_s* new_actor) {
    assert(root != nullptr, "Root cannot be null!");
    assert(new_actor != nullptr, "New actor to spawn cannot be null!");
    actor_s* end_ptr = Actor_list_end(root);
    end_ptr->next = new_actor;
    new_actor->prev = end_ptr;
}

void Actor_despawn(actor_s* to_despawn) {
    assert(to_despawn != nullptr, "Actor to despawn cannot be null!");
    actor_s* next_actor = to_despawn->next;
    actor_s* prev_actor = to_despawn->prev;

    if(next_actor) next_actor->prev = prev_actor;
    if(prev_actor) prev_actor->next = next_actor;

    free(to_despawn);
}

actor_s* Actor_create(float x, float y, actor_def_s* actor_definition) {
    actor_s* new_actor = malloc(sizeof(actor_s));
    new_actor->next = nullptr;
    new_actor->prev = nullptr;
    new_actor->ticks_since_spawn = 0;
    new_actor->thinker = actor_definition->thinker;
    new_actor->init = actor_definition->init;
    new_actor->render = actor_definition->render;
    new_actor->x = x;
    new_actor->y = y;
    return new_actor;
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

void Actor_register_def(char* actor_id, actor_init(actor_init), actor_thinker(actor_think), actor_render(actor_render)) {
    if(!actor_think || !actor_init || !actor_render || !actor_id) return;

    actor_def_s* actor_def = malloc(sizeof(actor_def_s));
    actor_def->thinker = actor_think;
    actor_def->init = actor_init;
    actor_def->render = actor_render;
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

    Actor_register_def("test_actor", &debug_actor_init, &debug_actor_think, &debug_actor_render);
}