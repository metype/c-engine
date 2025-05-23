#include "actor.h"
#include "../errors.h"
#include <malloc.h>

void A_tick(actor* root) {
    assert(root != nullptr, "Non-null root");
    actor* cur_actor = root;
    do {
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

actor* A_make_actor(float x, float y, void (*thinker)(actor*)) {
    actor* new_actor = malloc(sizeof(actor));
    new_actor->next = nullptr;
    new_actor->ticks_since_spawn = 0;
    new_actor->thinker = thinker;
    new_actor->x = x;
    new_actor->y = y;
    return new_actor;
}