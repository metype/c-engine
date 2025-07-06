#include "editor_actor_helper_funcs.h"


void set_cursor(SDL_Cursor* cursor, editor_actor_data_s* data) {
    if(cursor != data->pointer) data->used_cursor = true;
    SDL_SetCursor(cursor);
}

extern void setup_scene_vp(editor_actor_data_s* data);
extern void draw_actor_tree(float posX, float* posY, actor_s* root, editor_actor_data_s* data);
extern void draw_tree(editor_actor_data_s* data);
extern void draw_panels(editor_actor_data_s* data);
extern void draw_inspector(editor_actor_data_s* data);
extern void draw_actor_bbs(actor_s* root, editor_actor_data_s* data);
