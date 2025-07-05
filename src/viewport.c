#include "viewport.h"
#include "serialization.h"
#include "log.h"
#include "SDL3/SDL_render.h"
#include <string.h>

string* Viewport_serialize(void* serialized_obj) {
    if(!serialized_obj) return s("nil");
    viewport * vp = serialized_obj;
    string* ret_str = s("");

    generic_serialize_value(vp, type, int, "type");
    generic_serialize_value(vp, width, int, "width");
    generic_serialize_value(vp, height, int, "height");
    generic_serialize_value(vp, x, int, "x");
    generic_serialize_value(vp, y, int, "y");

    return ret_str;
}

void* Viewport_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    viewport* vp = malloc(sizeof(viewport));
    vp->texture = nullptr;
    vp->width = 0;
    vp->height = 0;
    vp->x = 0;
    vp->y = 0;

    generic_deserialize_begin("viewport")
        deserialize_stage_0()

        deserialize_stage_1()

        if (stage == 2) {
            snprintf(parser_buf, 128, "%s", buffer);
            stage = -1;
            if (strcmp(name_buf, "type") == 0) {
                int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                vp->type = *ptr;
                free(ptr);
                used = true;
            }

            if (strcmp(name_buf, "width") == 0) {
                int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                vp->width = *ptr;
                free(ptr);
                used = true;
            }

            if (strcmp(name_buf, "height") == 0) {
                int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                vp->height = *ptr;
                free(ptr);
                used = true;
            }

            if (strcmp(name_buf, "x") == 0) {
                int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                vp->x = *ptr;
                free(ptr);
                used = true;
            }

            if (strcmp(name_buf, "y") == 0) {
                int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                vp->y = *ptr;
                free(ptr);
                used = true;
            }

            if (!used) {
                Log_printf(LOG_LEVEL_WARNING, "Key %s not used in object %s, but referenced in file?", name_buf,
                           deserializer_name);
                used = false;
            }
        }
    generic_deserialize_end()

    vp->texture = nullptr;

    return vp;
}

viewport** viewport_stack = nullptr;
int active_vp = 0;

void Viewport_init(SDL_Renderer* renderer, viewport* vp) {
    if(!vp) return;
    if(vp->texture) SDL_DestroyTexture(vp->texture);
    vp->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, vp->width, vp->height);
    vp->render_target = renderer;
    switch(vp->type) {
        case VIEWPORT_SCALE_LINEAR:
        case VIEWPORT_SCALE_KEEP_LINEAR:
            SDL_SetTextureScaleMode(vp->texture, SDL_SCALEMODE_NEAREST);
            break;
        default:
            break;
    }
    SDL_SetTextureBlendMode(vp->texture, SDL_BLENDMODE_BLEND);
}

void Viewport_use(viewport* vp) {
    if(!viewport_stack) {
        viewport_stack = malloc(sizeof(viewport*) * 64);
    }

    if(!vp) return;
    /* We've gotta use some kind of renderer here, Active is probably fine? */
    if(!vp->texture) Viewport_init(Active_renderer(), vp);

    switch(vp->type) {
        case VIEWPORT_NO_SCALE:
        {
            int width;
            int height;
            SDL_GetWindowSize(SDL_GetRenderWindow(vp->render_target), &width, &height);
            if(width != vp->texture->w || height != vp->texture->h) {
                SDL_DestroyTexture(vp->texture);
                vp->width = width;
                vp->height = height;
                Viewport_init(vp->render_target, vp);
                break;
            }
        }
        default:
            break;
    }

    SDL_SetRenderTarget(vp->render_target, vp->texture);
    SDL_SetRenderDrawColor(vp->render_target, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
    SDL_RenderClear(vp->render_target);

    viewport_stack[active_vp++] = vp;
}

void Viewport_finish() {
    if(active_vp <= 0) {
        Log_print(LOG_LEVEL_ERROR, "Cannot finish root viewport!");
        return;
    }

    viewport* current_vp = viewport_stack[--active_vp];
    viewport* new_vp = viewport_stack[active_vp-1];

    if(new_vp->type == VIEWPORT_NO_SCALE) {
        int width;
        int height;
        SDL_GetWindowSize(SDL_GetRenderWindow(new_vp->render_target), &width, &height);
        if(width != new_vp->texture->w || height != new_vp->texture->h) {
            SDL_DestroyTexture(new_vp->texture);
            new_vp->width = width;
            new_vp->height = height;
            Viewport_init(new_vp->render_target, new_vp);
        }
    }

    SDL_SetRenderTarget(new_vp->render_target, new_vp->texture);

    SDL_FRect vp_rect = {
            .x = (float) current_vp->x,
            .y = (float) current_vp->y,
            .w = (float) current_vp->width,
            .h = (float) current_vp->height,
    };

    switch(current_vp->type) {
        case VIEWPORT_NO_SCALE:
        case VIEWPORT_SCALE:
        case VIEWPORT_SCALE_LINEAR:
            SDL_RenderTexture(current_vp->render_target, current_vp->texture, nullptr, &vp_rect);
            break;
        case VIEWPORT_SCALE_KEEP:
        case VIEWPORT_SCALE_KEEP_LINEAR:
        {
            int width = new_vp->width;
            int height = new_vp->height;

            float swidth = (float) width;
            float sheight = (float) height;

            float ratio = (float)current_vp->width / (float)current_vp->height;
            float final_width = sheight * ratio;
            float final_height = (float)height;
            if(final_width > swidth) {
                final_height *= swidth / final_width;
                final_width = swidth;
            }
            SDL_FRect render_rect = {
                    .w = final_width,
                    .h = final_height,
                    .x = ((swidth - final_width) / 2) + vp_rect.x,
                    .y = ((sheight - final_height) / 2) + vp_rect.y,
            };

            SDL_RenderTexture(current_vp->render_target, current_vp->texture, nullptr, &render_rect);
            break;
        }
        default:
            break;
    }
}

viewport* Viewport_active() {
    return viewport_stack[active_vp-1];
}

void Viewport_get_active_size(int* width, int* height) {
    viewport* current_vp = Viewport_active();
    if(!current_vp) return;
    if(width) *width = current_vp->width;
    if(height) *height = current_vp->height;
}

SDL_Renderer* Active_renderer() {
    return Viewport_active()->render_target;
}

//void Viewport_finish(SDL_Renderer* renderer, viewport* vp);
