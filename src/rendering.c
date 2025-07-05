#include "rendering.h"
#include "math.h"
#include "log.h"
#include <SDL3/SDL.h>
#include <math.h>
#include "actors/actor.h"

#include "definitions.h"
#include "engine.h"
#include "scene.h"

#if CENGINE_LINUX
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include "errors.h"
#include "viewport.h"
#include "gui.h"

#elif CENGINE_WIN32
#include "win32_stdlib.h"
#endif

SDL_Texture *game_surface_0 = nullptr, *game_surface_1 = nullptr, *game_surface_2 = nullptr, *game_surface_3 = nullptr;
SDL_Texture *ui_surface_0 = nullptr, *ui_surface_1 = nullptr, *ui_surface_2 = nullptr;

uint8_t layers_in_use = 0;

SDL_Texture *canvas_3d = nullptr;

#define R_WIDTH 640
#define R_HEIGHT 360
#define R_PIXEL_COUNT (R_WIDTH * R_HEIGHT * 4)

void R_switch_layer(SDL_Renderer* renderer, uint8_t layer) {
    if(layer != LAYER_BASE) {
        SDL_Texture** surfaces[8] = {nullptr, &game_surface_0, &game_surface_1, &game_surface_2, &game_surface_3, &ui_surface_0, &ui_surface_1, &ui_surface_2};

        int idx = (int)log2(layer);
        SDL_Texture** surface = surfaces[idx];

        if(!*surface) {
            int w, h;
            SDL_GetCurrentRenderOutputSize(renderer, &w, &h);
            *surface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, w, h);
            SDL_SetTextureBlendMode(*surface, SDL_BLENDMODE_BLEND);
        }

        SDL_SetRenderTarget(renderer, *surface);

        if((layers_in_use & layer) == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
            SDL_RenderClear(renderer);
        }
    } else {
        SDL_SetRenderTarget(renderer, nullptr);
        if((layers_in_use & layer) == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);
        }
    }

    layers_in_use |= layer;
}

void R_render_composite(SDL_Renderer* renderer) {
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_Texture* surfaces[7] = {game_surface_0, game_surface_1, game_surface_2, game_surface_3, ui_surface_0, ui_surface_1, ui_surface_2};
    for(int i = 0; i < 7; i++) {
//        if(!(layers_in_use & (1 << (i + 1)))) continue;
        SDL_RenderTexture(renderer, surfaces[i], nullptr, nullptr);
    }
    layers_in_use = 0;
}

scene* R_init_scene(SDL_Renderer* renderer) {
    if(!canvas_3d) {
        canvas_3d = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, R_WIDTH, R_HEIGHT);
        SDL_SetTextureBlendMode(canvas_3d, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(canvas_3d, SDL_SCALEMODE_NEAREST);
    }
    srandom(time(nullptr));

    scene* new_scene = malloc(sizeof(scene));

    return new_scene;
}

void R_render_scene(app_state_s* state_ptr, volatile scene* scene_to_render) {
    if(!scene_to_render->base_vp) return;
    if(!scene_to_render->base_vp->texture) Viewport_init(state_ptr->renderer_ptr, state_ptr->scene->base_vp);

    Viewport_use(scene_to_render->base_vp);

    assert(scene_to_render->actor_tree != nullptr, "Tried to render null scene!", return);
    Actor_render((actor_s*) scene_to_render->actor_tree, state_ptr);

    Gui_render(state_ptr->renderer_ptr);
    Viewport_finish();
}