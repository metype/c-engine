#include "actor.h"
#include "../log.h"
#include "../errors.h"
#include "../hashmap.h"
#include "test_actor.h"
#include <malloc.h>
#include <pthread.h>

void A_tick(actor* root) {
    assert(root != nullptr, "Non-null root");
    actor* cur_actor = root;
    do {
        if(cur_actor->init && cur_actor->ticks_since_spawn == 0) cur_actor->init(cur_actor);
        if(cur_actor->thinker) cur_actor->thinker(cur_actor);
        cur_actor->ticks_since_spawn++;
    } while( (cur_actor = cur_actor->next) );
}

struct actor* A_get_end_ptr(actor* root) {
    assert(root != nullptr, "Non-null root");
    actor* cur_actor = root;
    do {
        cur_actor = cur_actor->next;
    } while(cur_actor->next);
    return cur_actor;
}

void A_spawn(actor* root, actor* new_actor) {
    assert(root != nullptr, "Non-null root");
    assert(new_actor != nullptr, "Non-null spawned actor");
    new_actor->next = nullptr;
    actor* end_ptr = A_get_end_ptr(root);
    end_ptr->next = new_actor;
}

void A_despawn(actor* root, actor* to_despawn) {
    assert(root != nullptr, "Non-null root");
    assert(to_despawn != nullptr, "Non-null despawned actor");
    actor* cur_actor = root;

    do {
        cur_actor = cur_actor->next;
    } while(cur_actor->next != to_despawn);

    cur_actor->next = to_despawn->next;
}

actor* A_make_actor(float x, float y, actor_def* actor_definition) {
    actor* new_actor = malloc(sizeof(actor));
    new_actor->next = nullptr;
    new_actor->ticks_since_spawn = 0;
    new_actor->thinker = actor_definition->thinker;
    new_actor->init = actor_definition->init;
    new_actor->render = actor_definition->render;
    new_actor->x = x;
    new_actor->y = y;
    return new_actor;
}

hash_map* actor_defs = nullptr;
pthread_mutex_t actor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t actor_mutex_attr;

actor_def* A_get_actor_def(char* actor_id) {
    pthread_mutex_lock(&actor_mutex);
    if(!actor_defs) {
        pthread_mutex_unlock(&actor_mutex);
        A_register_actor_def(nullptr, nullptr);
        pthread_mutex_lock(&actor_mutex);
    }
    actor_def* def = map_get(actor_defs, actor_id, actor_def*);
    pthread_mutex_unlock(&actor_mutex);
    return def;
}

void A_register_actor_def(char* actor_id, actor_def* actor_def) {
    pthread_mutex_lock(&actor_mutex);
    if(!actor_defs) {
        actor_defs = malloc(sizeof(hash_map));
        map_init(actor_defs);
    }

    if(actor_id == nullptr || actor_def == nullptr) return;

    map_set(actor_defs, actor_id, actor_def);
    pthread_mutex_unlock(&actor_mutex);
}

void A_register_default_actors() {
    pthread_mutex_init(&actor_mutex, &actor_mutex_attr);
    actor_def* test_actor = malloc(sizeof(actor_def));
    test_actor->thinker = &debug_actor_think;
    test_actor->init = &debug_actor_init;
    test_actor->render = &debug_actor_render;
    A_register_actor_def("test_actor", test_actor);
}