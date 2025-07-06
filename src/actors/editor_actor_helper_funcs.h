#ifndef CENGINE_EDITOR_ACTOR_HELPER_FUNCS_H
#define CENGINE_EDITOR_ACTOR_HELPER_FUNCS_H
#include <math.h>

#include "editor_actor.h"
#include "viewport_actor.h"

#include "../viewport.h"
#include "../input.h"
#include "../util.h"

void set_cursor(SDL_Cursor* cursor, editor_actor_data_s* data);

inline void setup_scene_vp(editor_actor_data_s* data) {
    int width;
    int height;
    SDL_GetWindowSize(SDL_GetRenderWindow(Active_renderer()), &width, &height);
    data->edited_scene->base_vp->width = width - (data->left_panel_size + data->right_panel_size);
    data->edited_scene->base_vp->height = height - (data->top_panel_size + data->bottom_panel_size);
    data->edited_scene->base_vp->x = data->left_panel_size;
    data->edited_scene->base_vp->y = data->top_panel_size;
    if(data->edited_scene->base_vp->texture) SDL_DestroyTexture(data->edited_scene->base_vp->texture);
    data->edited_scene->base_vp->texture = nullptr;
}

inline void draw_actor_tree(float posX, float* posY, actor_s* root, editor_actor_data_s* data) { // NOLINT(*-no-recursion)
    char* str = malloc(sizeof(char) * 512);
    sprintf(str, "%s (%s)", root->name, root->actor_id);

    SDL_FRect bounding_rect = {
            .x = posX - 2,
            .y = (*posY) - 2,
            .w = strlen(str) * 8 + 4,
            .h = 12,
    };

    float mx, my;
    SDL_GetMouseState(&mx, &my);

    if(mx > bounding_rect.x && mx < bounding_rect.x + bounding_rect.w && my > bounding_rect.y && my < bounding_rect.y + bounding_rect.h) {
        set_cursor(data->select, data);
        if(Input_mouse_button_just_pressed(SDL_BUTTON_LEFT)) {
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
        SDL_RenderLine(Active_renderer(), xPos, startingY + 10, xPos, endingY - 4);
    }
}

inline void draw_tree(editor_actor_data_s* data) {

    float mx;
    float my;
    SDL_GetMouseState(&mx, &my);

    float yPos = 28;

    SDL_Renderer* renderer = Active_renderer();

    SDL_Rect tree_clipper = {
            .x = 4,
            .y = yPos - 4,
            .h = 512,
            .w = data->left_panel_size - 8,
    };

    SDL_FRect tree_bg = {};

    SDL_RectToFRect(&tree_clipper, &tree_bg);


    SDL_SetRenderDrawColor(renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);

    SDL_RenderFillRect(renderer, &tree_bg);

    SDL_SetRenderDrawColor(renderer, 240, 240, 240, SDL_ALPHA_OPAQUE);

    SDL_RenderLine(renderer, tree_bg.x, tree_bg.y + tree_bg.h, tree_bg.x + tree_bg.w, tree_bg.y + tree_bg.h);
    SDL_RenderLine(renderer, tree_bg.x + tree_bg.w, tree_bg.y, tree_bg.x + tree_bg.w, tree_bg.y + tree_bg.h);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    SDL_RenderLine(renderer, tree_bg.x, tree_bg.y, tree_bg.x + tree_bg.w, tree_bg.y);
    SDL_RenderLine(renderer, tree_bg.x, tree_bg.y, tree_bg.x, tree_bg.y + tree_bg.h);

    if(mx > tree_clipper.x && mx < tree_clipper.x + tree_clipper.w && my > tree_clipper.y && my < tree_clipper.y + tree_clipper.h) {
        if(Input_mouse_button_just_pressed(SDL_BUTTON_LEFT)) {
            data->selected_actor = nullptr;
        }
    }

    SDL_SetRenderClipRect(renderer, &tree_clipper);
    draw_actor_tree(8, &yPos, (actor_s*) data->edited_scene->actor_tree, data);
    SDL_SetRenderClipRect(renderer, nullptr);
}

inline void draw_inspector(editor_actor_data_s* data) {
    float mx;
    float my;
    SDL_GetMouseState(&mx, &my);

    float yPos = 28;

    SDL_Renderer* renderer = Active_renderer();

    int wwidth;
    SDL_GetWindowSize(SDL_GetRenderWindow(renderer), &wwidth, nullptr);

    SDL_Rect inspector_clipper = {
            .x = wwidth - data->right_panel_size + 4,
            .y = yPos - 4,
            .h = 512,
            .w = data->right_panel_size - 8,
    };

    SDL_FRect inspector_bg = {};

    SDL_RectToFRect(&inspector_clipper, &inspector_bg);

    SDL_SetRenderDrawColor(renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);

    SDL_RenderFillRect(renderer, &inspector_bg);

    SDL_SetRenderDrawColor(renderer, 240, 240, 240, SDL_ALPHA_OPAQUE);

    SDL_RenderLine(renderer, inspector_bg.x, inspector_bg.y + inspector_bg.h, inspector_bg.x + inspector_bg.w, inspector_bg.y + inspector_bg.h);
    SDL_RenderLine(renderer, inspector_bg.x + inspector_bg.w, inspector_bg.y, inspector_bg.x + inspector_bg.w, inspector_bg.y + inspector_bg.h);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    SDL_RenderLine(renderer, inspector_bg.x, inspector_bg.y, inspector_bg.x + inspector_bg.w, inspector_bg.y);
    SDL_RenderLine(renderer, inspector_bg.x, inspector_bg.y, inspector_bg.x, inspector_bg.y + inspector_bg.h);

    SDL_SetRenderClipRect(renderer, &inspector_clipper);
    if(data->selected_actor) {
        actor_property_s* prop = data->selected_actor->properties;
        int y = inspector_clipper.y + 4;
        char* active_category;
        while(prop) {
            int x = inspector_bg.x + 4;
            char* this_name = strdup(prop->name);
            for(int i = 0; i < strlen(prop->name); i++) {
                if(prop->name[i] == '.') {
                    x += 8;
                }
                SDL_RenderDebugText(renderer, x, y, this_name);
            }
            SDL_RenderDebugText(renderer, x, y, prop->name);
            y += 12;
            prop = prop->next;
        }
    }
    SDL_SetRenderClipRect(renderer, nullptr);
}

inline void draw_panels(editor_actor_data_s* data) {
    float mx;
    float my;
    SDL_MouseButtonFlags mb = SDL_GetMouseState(&mx, &my);

    SDL_Renderer* renderer = Active_renderer();

    int wwidth;
    int wheight;
    SDL_GetWindowSize(SDL_GetRenderWindow(renderer), &wwidth, &wheight);

    SDL_FRect left_panel = {
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
            data->left_panel_size = Maxf((int)mx, 200);
            data->lp_resize = true;
        } else if (!(mb & SDL_BUTTON_LEFT)) {
            setup_scene_vp(data);
            data->lp_resize = false;
        }
    } else if(fabsf(mx - right_panel.x) < 5 || data->rp_resize) {
        set_cursor(data->resize_we, data);
        if((mb & SDL_BUTTON_LEFT)) {
            data->right_panel_size = Maxf(wwidth - (int)mx, 200);
            data->rp_resize = true;
        } else if (!(mb & SDL_BUTTON_LEFT)) {
            setup_scene_vp(data);
            data->rp_resize = false;
        }
    } else if((fabsf(my - bottom_panel.y) < 5 && mx > left_panel.x + left_panel.w && mx < right_panel.x) || data->bp_resize) {
        set_cursor(data->resize_ns, data);
        if((mb & SDL_BUTTON_LEFT)) {
            data->bottom_panel_size = Maxf(wheight - (int)my, 256);
            data->bp_resize = true;
        } else if (!(mb & SDL_BUTTON_LEFT)) {
            setup_scene_vp(data);
            data->bp_resize = false;
        }
    }

    SDL_SetRenderDrawColor(renderer, 210, 210, 210, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &left_panel);
    SDL_RenderFillRect(renderer, &right_panel);
    SDL_RenderFillRect(renderer, &top_panel);
    SDL_RenderFillRect(renderer, &bottom_panel);

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderLine(renderer, left_panel.x + left_panel.w, left_panel.y + top_panel.h, left_panel.x + left_panel.w, left_panel.y + left_panel.h);
    SDL_RenderLine(renderer, right_panel.x, right_panel.y + top_panel.h, right_panel.x, right_panel.y + right_panel.h);
    SDL_RenderLine(renderer, top_panel.x, top_panel.y + top_panel.h, top_panel.x + top_panel.w, top_panel.y + top_panel.h);
    SDL_RenderLine(renderer, left_panel.x + left_panel.w, bottom_panel.y, right_panel.x, bottom_panel.y);

    draw_tree(data);
    draw_inspector(data);
}

inline void draw_actor_bbs(actor_s* root, editor_actor_data_s* data) { // NOLINT(*-no-recursion)
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

#endif //CENGINE_EDITOR_ACTOR_HELPER_FUNCS_H
