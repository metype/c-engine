#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include "editor_actor.h"
#include "../log.h"
#include "../util.h"
#include "../audio.h"
#include "../viewport.h"
#include "../scene.h"
#include "../rendering.h"
#include "viewport_actor.h"

void setup_scene_vp(app_state_s* state_ptr, editor_actor_data_s* data) {
    int width;
    int height;
    SDL_GetWindowSize(state_ptr->window_ptr, &width, &height);
    data->edited_scene->base_vp->width = width - (data->left_panel_size + data->right_panel_size);
    data->edited_scene->base_vp->height = height - (data->top_panel_size + data->bottom_panel_size);
    data->edited_scene->base_vp->x = data->left_panel_size;
    data->edited_scene->base_vp->y = data->top_panel_size;
    if(data->edited_scene->base_vp->texture) SDL_DestroyTexture(data->edited_scene->base_vp->texture);
    data->edited_scene->base_vp->texture = nullptr;
}

void draw_actor_tree(float posX, float* posY, actor_s* root, editor_actor_data_s* data);
void draw_actor_bbs(actor_s* root, editor_actor_data_s* data);

bool used_cursor;

void set_cursor(SDL_Cursor* cursor, editor_actor_data_s* data);

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
        setup_scene_vp(state_ptr, data);
        data->selected_actor = Actor_from_path_rel(data->edited_scene->actor_tree, "tile_map/tile_map_child");
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

    used_cursor = false;

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

    float mx;
    float my;
    SDL_MouseButtonFlags mb = SDL_GetMouseState(&mx, &my);

    SDL_FRect left_panel = {
            .x = 0,
            .y = 0,
            .h = wheight,
            .w = data->left_panel_size,
    };

    SDL_Rect left_panel_clipper = {
            .x = 0,
            .y = 0,
            .h = wheight,
            .w = data->left_panel_size,
    };

    SDL_FRect right_panel = {
            .x = wwidth - data->right_panel_size,
            .y = 0,
            .h = wheight,
            .w = data->right_panel_size,
    };

    SDL_FRect top_panel = {
            .x = 0,
            .y = 0,
            .h = data->top_panel_size,
            .w = wwidth,
    };

    SDL_FRect bottom_panel = {
            .x = data->left_panel_size,
            .y = wheight - data->bottom_panel_size,
            .h = data->bottom_panel_size,
            .w = wwidth - (data->right_panel_size + data->left_panel_size),
    };

    if(fabsf(mx - (left_panel.x + left_panel.w)) < 5 || data->lp_resize) {
        set_cursor(data->resize_we, data);
        if((mb & SDL_BUTTON_LEFT)) {
            data->left_panel_size = (int)mx;
            data->lp_resize = true;
        } else if (!(mb & SDL_BUTTON_LEFT)) {
            setup_scene_vp(state_ptr, data);
            data->lp_resize = false;
        }
    } else if(fabsf(mx - right_panel.x) < 5 || data->rp_resize) {
        set_cursor(data->resize_we, data);
        if((mb & SDL_BUTTON_LEFT)) {
            data->right_panel_size = wwidth - (int)mx;
            data->rp_resize = true;
        } else if (!(mb & SDL_BUTTON_LEFT)) {
            setup_scene_vp(state_ptr, data);
            data->rp_resize = false;
        }
    } else if((fabsf(my - bottom_panel.y) < 5 && mx > left_panel.x + left_panel.w && mx < right_panel.x) || data->bp_resize) {
        set_cursor(data->resize_ns, data);
        if((mb & SDL_BUTTON_LEFT)) {
            data->bottom_panel_size = wheight - (int)my;
            data->bp_resize = true;
        } else if (!(mb & SDL_BUTTON_LEFT)) {
            setup_scene_vp(state_ptr, data);
            data->bp_resize = false;
        }
    }

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 210, 210, 210, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(state_ptr->renderer_ptr, &left_panel);
    SDL_RenderFillRect(state_ptr->renderer_ptr, &right_panel);
    SDL_RenderFillRect(state_ptr->renderer_ptr, &top_panel);
    SDL_RenderFillRect(state_ptr->renderer_ptr, &bottom_panel);

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 20, 20, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderLine(state_ptr->renderer_ptr, left_panel.x + left_panel.w, left_panel.y + top_panel.h, left_panel.x + left_panel.w, left_panel.y + left_panel.h);
    SDL_RenderLine(state_ptr->renderer_ptr, right_panel.x, right_panel.y + top_panel.h, right_panel.x, right_panel.y + right_panel.h);
    SDL_RenderLine(state_ptr->renderer_ptr, top_panel.x, top_panel.y + top_panel.h, top_panel.x + top_panel.w, top_panel.y + top_panel.h);
    SDL_RenderLine(state_ptr->renderer_ptr, left_panel.x + left_panel.w, bottom_panel.y, right_panel.x, bottom_panel.y);

    SDL_SetRenderDrawColor(state_ptr->renderer_ptr, 0, 0, 0, SDL_ALPHA_OPAQUE);
    float yPos = 28;

    SDL_SetRenderClipRect(state_ptr->renderer_ptr, &left_panel_clipper);
    draw_actor_tree(8, &yPos, (actor_s*) data->edited_scene->actor_tree, data);
    SDL_SetRenderClipRect(state_ptr->renderer_ptr, nullptr);
    if(!used_cursor) {
        SDL_SetCursor(data->pointer);
    }
}

void draw_actor_tree(float posX, float* posY, actor_s* root, editor_actor_data_s* data) { // NOLINT(*-no-recursion)
    char* str = malloc(sizeof(char) * 512);
    sprintf(str, "%s (%s)", root->name, root->actor_id);

    SDL_FRect bounding_rect = {
            .x = posX - 2,
            .y = (*posY) - 2,
            .w = strlen(str) * 8 + 4,
            .h = 12,
    };

    float mx, my;

    SDL_MouseButtonFlags mb = SDL_GetMouseState(&mx, &my);

    if(mx > bounding_rect.x && mx < bounding_rect.x + bounding_rect.w && my > bounding_rect.y && my < bounding_rect.y + bounding_rect.h) {
        set_cursor(data->select, data);
        if(mb & SDL_BUTTON_LEFT) {
            data->selected_actor = root;
        }
    }

    if(root == data->selected_actor) {
        Uint8 r, g, b;

        SDL_GetRenderDrawColor(Active_renderer(), &r, &g, &b, nullptr);
        SDL_SetRenderDrawColor(Active_renderer(), 150, 195, 239, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(Active_renderer(), &bounding_rect);
        SDL_SetRenderDrawColor(Active_renderer(), r, g, b, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderDebugText(Active_renderer(), posX, *posY, str);
    free(str);
    if(root->children && root->children->element_count > 0) {
        float startingY = *posY;
        float endingY;
        float xPos = posX + 3;
        for(int i = 0; i < root->children->element_count; i++) {
            *posY += 12;
            endingY = *posY + 8;
            float yPos = *posY + 4;
            SDL_RenderLine(Active_renderer(), xPos, yPos, xPos + 6, yPos);
            draw_actor_tree(posX + 12, posY, root->children->array[i], data);
        }
        SDL_RenderLine(Active_renderer(), xPos, startingY + 12, xPos, endingY - 4);
    }
}

void draw_actor_bbs(actor_s* root, editor_actor_data_s* data) { // NOLINT(*-no-recursion)
    if(root == data->selected_actor) {
        rect bb = Actor_get_bounding_box(root);
        int w, h;
        Viewport_get_active_size(&w, &h);
        bb.tl.x = Maxf(bb.tl.x, 1);
        bb.tl.y = Maxf(bb.tl.y, 1);
        bb.br.x = Minf(bb.br.x, w);
        bb.br.y = Minf(bb.br.y, h);
        SDL_FRect sdl_bb = {
                .x = bb.tl.x,
                .y = bb.tl.y,
                .w = bb.br.x - bb.tl.x,
                .h = bb.br.y - bb.tl.y,
        };

        SDL_RenderRect(Active_renderer(), &sdl_bb);
    }

    if(root->children && root->children->element_count > 0) {
        for(int i = 0; i < root->children->element_count; i++) {
            actor_s* child_ptr = root->children->array[i];

            if(strcmp(child_ptr->actor_id, "viewport") == 0) {
                perf_metrics_s metrics = {.time_in_tick = 1};
                app_state_s state = {.perf_metrics_ptr = &metrics, .renderer_ptr = Active_renderer()};

                Uint8 r, g, b;

                SDL_GetRenderDrawColor(Active_renderer(), &r, &g, &b, nullptr);

                viewport_actor_render(child_ptr, &state);

                SDL_SetRenderDrawColor(Active_renderer(), r, g, b, SDL_ALPHA_OPAQUE);

                draw_actor_bbs(child_ptr, data);

                viewport_actor_late_render(child_ptr, &state);
                continue;
            }

            draw_actor_bbs(child_ptr, data);
        }
    }
}

void editor_actor_late_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, editor_actor_data_s);
}

void editor_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event) {
    ACTOR_PRE_EVENT(self, editor_actor_data_s);

    switch(event->type) {
        case SDL_EVENT_WINDOW_RESIZED:
            setup_scene_vp(state_ptr, data);
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

void set_cursor(SDL_Cursor* cursor, editor_actor_data_s* data) {
    if(cursor != data->pointer) used_cursor = true;
    SDL_SetCursor(cursor);
}