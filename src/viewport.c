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

    return ret_str;
}

void* Viewport_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    viewport* vp = malloc(sizeof(viewport));

    generic_deserialize_begin("viewport")
        deserialize_stage_0()

        deserialize_stage_1()

        if (stage == 2) {
            snprintf(parser_buf, 128, "%s", buffer);
            stage = -1;
            {
                do {
                    if (strcmp(name_buf, "type") == 0) {
                        int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                        vp->type = *ptr;
                        free(ptr);
                        used = true;
                    }
                }
                while (0);
                do {
                    if (strcmp(name_buf, "width") == 0) {
                        int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                        vp->width = *ptr;
                        free(ptr);
                        used = true;
                    }
                }
                while (0);
                do {
                    if (strcmp(name_buf, "height") == 0) {
                        int *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                        vp->height = *ptr;
                        free(ptr);
                        used = true;
                    }
                }
                while (0);
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

viewport* used_vp = nullptr;

void Viewport_init(SDL_Renderer* renderer, viewport* vp) {
    if(!vp) return;
    vp->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, vp->width, vp->height);
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

void Viewport_use(SDL_Renderer* renderer, viewport* vp) {
    if(used_vp) {
        switch(used_vp->type) {
            case VIEWPORT_NO_SCALE:
            {
                int width;
                int height;
                SDL_GetWindowSize(SDL_GetRenderWindow(renderer), &width, &height);
                if(width != used_vp->texture->w || height != used_vp->texture->h) {
                    SDL_DestroyTexture(used_vp->texture);
                    used_vp->width = width;
                    used_vp->height = height;
                    Viewport_init(renderer, used_vp);
                    break;
                }
            }
            case VIEWPORT_SCALE:
            case VIEWPORT_SCALE_LINEAR:
                SDL_SetRenderTarget(renderer, nullptr);
                SDL_RenderTexture(renderer, used_vp->texture, nullptr, nullptr);
                SDL_SetRenderTarget(renderer, used_vp->texture);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
                SDL_RenderClear(renderer);
                break;
            case VIEWPORT_SCALE_KEEP:
            case VIEWPORT_SCALE_KEEP_LINEAR:
            {
                int width;
                int height;
                SDL_GetWindowSize(SDL_GetRenderWindow(renderer), &width, &height);

                float swidth = (float) width;
                float sheight = (float) height;

                float ratio = (float)used_vp->texture->w / (float)used_vp->texture->h;
                float final_width = sheight * ratio;
                float final_height = (float)height;
                if(final_width > swidth) {
                    final_height *= swidth / final_width;
                    final_width = swidth;
                }
                SDL_FRect render_rect = {
                        .w = final_width,
                        .h = final_height,
                        .x = (swidth - final_width) / 2,
                        .y = (sheight - final_height) / 2,
                };
                SDL_SetRenderTarget(renderer, nullptr);
                SDL_RenderTexture(renderer, used_vp->texture, nullptr, &render_rect);
                SDL_SetRenderTarget(renderer, used_vp->texture);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
                SDL_RenderClear(renderer);
                break;
            }
            default:
                break;
        }
    }

    if(!vp) return;

    if(!vp->texture) {
        Viewport_init(renderer, vp);
    }

    SDL_SetRenderTarget(renderer, vp->texture);

    used_vp = vp;
}

viewport* Viewport_active() {
    return used_vp;
}

//void Viewport_finish(SDL_Renderer* renderer, viewport* vp);
