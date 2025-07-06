#include <stdlib.h>

#include "editor_actor.h"
#include "editor_actor_helper_funcs.h"

#include "../rendering.h"

void editor_actor_init(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_INIT(self, editor_actor_data_s);

    data->top_panel_size = 20;
    data->left_panel_size = 256;
    data->right_panel_size = 256;
    data->bottom_panel_size = 312;

    data->pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    data->resize_we = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
    data->resize_ns = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
    data->select = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);

    data->selected_actor = nullptr;

    if(data->edited_scene) {
        if(data->edited_scene->actor_tree) data->edited_scene->actor_tree->process = false;
        if(!data->edited_scene->base_vp) {
            data->edited_scene->base_vp = malloc(sizeof(viewport));
        }
        data->edited_scene->base_vp->type = VIEWPORT_SCALE;
        setup_scene_vp(data);
//        data->selected_actor = Actor_from_path_rel(data->edited_scene->actor_tree, "tile_map/tile_map_child");
    }
}

void editor_actor_think(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_THINK(self, editor_actor_data_s);

    Actor_update_transforms(data->edited_scene->actor_tree);
    Actor_tick(data->edited_scene->actor_tree, state_ptr);
}

void editor_actor_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, editor_actor_data_s);
    if(!data->edited_scene) return;

    data->used_cursor = false;

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(state_ptr->renderer_ptr, 8, 8, "adsadasadasd");

    if(!data->edited_scene->base_vp->texture) Viewport_init(state_ptr->renderer_ptr, data->edited_scene->base_vp);

    int wwidth;
    int wheight;
    SDL_GetWindowSize(state_ptr->window_ptr, &wwidth, &wheight);


    if(!data->bp_resize && !data->lp_resize && !data->rp_resize) {
        R_render_scene(state_ptr, data->edited_scene);
        Viewport_use(data->edited_scene->base_vp);

        SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 200, 130, 20, SDL_ALPHA_OPAQUE);
        draw_actor_bbs((actor_s*) data->edited_scene->actor_tree, data);

        Viewport_finish();
    } else {
        SDL_FRect clear_rect = {
                .w = wwidth - (data->left_panel_size + data->right_panel_size),
                .h = wheight - (data->top_panel_size + data->bottom_panel_size),
                .x = data->left_panel_size,
                .y = data->top_panel_size
        };

        SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 120, 0, 200, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(state_ptr->renderer_ptr, &clear_rect);
        SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDebugText(state_ptr->renderer_ptr, (clear_rect.w / 2) + clear_rect.x - 44, (clear_rect.h / 2) + clear_rect.y - 4, "Resizing...");
    }

    draw_panels(data);
    if(!data->used_cursor) {
        SDL_SetCursor(data->pointer);
    }
}

void editor_actor_late_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, editor_actor_data_s);
}

void editor_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event) {
    ACTOR_PRE_EVENT(self, editor_actor_data_s);

    switch(event->type) {
        case SDL_EVENT_WINDOW_RESIZED:
            setup_scene_vp(data);
            break;
    }
}

void editor_actor_recalc_bb(actor_s* self) {
    ACTOR_DATA_PTR(self, editor_actor_data_s);

    SDL_Window** windows = SDL_GetWindows(nullptr);
    if(!windows) return;
    int ww, wh;
    SDL_GetWindowSize(windows[0], &ww, &wh);

    if(self->bb) free(self->bb);
    self->bb = malloc(sizeof(rect));
    self->bb->tl = (float2_s) {.x = self->transform.position.x, .y = self->transform.position.y};
    self->bb->br = (float2_s) {.x = (float) ww, .y = (float) wh};
}


// The editor actor can't be saved really, but that's not a problem.
string* editor_actor_serialize(void* serialized_obj) {
    return s("nil");
}

void* editor_actor_deserialize(string* str) {
    return nullptr;
}

