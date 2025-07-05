#ifndef EDITOR_ACTOR_H
#define EDITOR_ACTOR_H
#include "actor.h"

typedef struct scene scene;
typedef struct viewport_s viewport;

typedef struct editor_actor_data_s {
    scene* edited_scene;
    int top_panel_size;
    int bottom_panel_size;
    int left_panel_size;
    int right_panel_size;
    SDL_Cursor* pointer;
    SDL_Cursor* resize_we;
    SDL_Cursor* resize_ns;
    SDL_Cursor* select;
    bool lp_resize;
    bool rp_resize;
    bool bp_resize;
    actor_s* selected_actor;
} editor_actor_data_s;

void editor_actor_init(actor_s* self, app_state_s* state_ptr);
void editor_actor_think(actor_s* self, app_state_s* state_ptr);
void editor_actor_render(actor_s* self, app_state_s* state_ptr);
void editor_actor_late_render(actor_s* self, app_state_s* state_ptr);
void editor_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event);
void editor_actor_recalc_bb(actor_s* self);

string* editor_actor_serialize(void* serialized_obj);
void* editor_actor_deserialize(string* str);

#endif //EDITOR_ACTOR_H