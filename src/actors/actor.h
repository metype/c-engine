#ifndef CPROJ_ACTOR_H
#define CPROJ_ACTOR_H
typedef struct actor {
    float x, y;
    int ticks_since_spawn;
    void (*thinker)(struct actor*);
    struct actor* next;
} actor;

void A_tick(actor* root);
actor* A_get_end_ptr(actor* root);
void A_spawn(actor* root, actor* new_actor);
void A_despawn(actor* root, actor* to_despawn);
actor* A_make_actor(float x, float y, void (*thinker)(actor*));

#endif //CPROJ_ACTOR_H
