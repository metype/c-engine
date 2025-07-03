#ifndef CENGINE_TILEMAP_ACTOR_H
#define CENGINE_TILEMAP_ACTOR_H
#include "actor.h"

typedef struct tilemap_actor_data_s {
    int tilemap_width;
    int tilemap_height;
    long** tilemap;

    bool up;
    bool left;
    bool down;
    bool right;

    int sWidth;
    int sHeight;
} tilemap_actor_data_s;

void tilemap_actor_init(actor_s* self, app_state_s* state_ptr);
void tilemap_actor_think(actor_s* self, app_state_s* state_ptr);
void tilemap_actor_render(actor_s* self, app_state_s* state_ptr);
void tilemap_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event);

string* tilemap_actor_serialize(void* serialized_obj);
void* tilemap_actor_deserialize(string* str);
#endif //CENGINE_TILEMAP_ACTOR_H
