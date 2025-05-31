#include "rendering.h"
#include "math.h"
#include <SDL3/SDL.h>
#include <math.h>

SDL_Texture *game_surface_0 = nullptr, *game_surface_1 = nullptr, *game_surface_2 = nullptr, *game_surface_3 = nullptr;
SDL_Texture *ui_surface_0 = nullptr, *ui_surface_1 = nullptr, *ui_surface_2 = nullptr;

uint8_t layers_in_use = 0;

SDL_Texture *canvas_3d = nullptr;

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

void R_render_scene(SDL_Renderer* renderer) {
    if(!canvas_3d) {
        canvas_3d = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 1280, 720);
        SDL_SetTextureBlendMode(canvas_3d, SDL_BLENDMODE_BLEND);
    }
    uint32_t* pixels;
    float width;
    float height;
    int pitch;
    SDL_GetTextureSize(canvas_3d, &width, &height);
    SDL_LockTexture(canvas_3d, nullptr, (void **) &pixels, &pitch);
    for(int x = 0; x < (int)width; x++) {
        for(int y = 0; y < (int)height; y++) {
            int idx = x + (y * (int)width);
            bool is_in_tri = point_in_triangle((float2_s){400, 600}, (float2_s){500, 700}, (float2_s){350, 700}, (float2_s){x, y}, nullptr);
            pixels[idx] = is_in_tri ? 0xFF0000FF : 0xFFFFFFFF;
        }
    }
    SDL_UnlockTexture(canvas_3d);
    SDL_RenderTexture(renderer, canvas_3d, nullptr, nullptr);
}