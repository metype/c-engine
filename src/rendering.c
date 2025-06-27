#include "rendering.h"
#include "math.h"
#include "log.h"
#include <SDL3/SDL.h>
#include <math.h>
#include "actors/actor.h"

#include "definitions.h"
#include "engine.h"

#if CENGINE_LINUX
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include "errors.h"

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

    const int triangle_count = 25;
    scene* new_scene = malloc(sizeof(scene));
//    new_scene->triangle_count = triangle_count;
//    new_scene->triangles = malloc(sizeof(tri) * triangle_count);
//    new_scene->triangle_colors = malloc(sizeof(uint32_t) * triangle_count);
//
//    float2_s half_size = (float2_s){350, 200};
//
//    for(int i = 0; i < triangle_count; i++) {
//        new_scene->triangles[i].a = float2_add(float2_mul_f(float2_rand_d(-200, 200, -200, 200), 0.3f), half_size);
//        new_scene->triangles[i].b = float2_add(float2_mul_f(float2_rand_d(-200, 200, -200, 200), 0.3f), half_size);
//        new_scene->triangles[i].c = float2_add(float2_mul_f(float2_rand_d(-200, 200, -200, 200), 0.3f), half_size);
//
//        float left = 2000000, top = 2000000, right = -2000000, bottom = -2000000;
//        if(new_scene->triangles[i].a.x < left) left = new_scene->triangles[i].a.x;
//        if(new_scene->triangles[i].a.x > right) right = new_scene->triangles[i].a.x;
//        if(new_scene->triangles[i].a.y < top) top = new_scene->triangles[i].a.y;
//        if(new_scene->triangles[i].a.y > bottom) bottom = new_scene->triangles[i].a.y;
//
//        if(new_scene->triangles[i].b.x < left) left = new_scene->triangles[i].b.x;
//        if(new_scene->triangles[i].b.x > right) right = new_scene->triangles[i].b.x;
//        if(new_scene->triangles[i].b.y < top) top = new_scene->triangles[i].b.y;
//        if(new_scene->triangles[i].b.y > bottom) bottom = new_scene->triangles[i].b.y;
//
//        if(new_scene->triangles[i].c.x < left) left = new_scene->triangles[i].c.x;
//        if(new_scene->triangles[i].c.x > right) right = new_scene->triangles[i].c.x;
//        if(new_scene->triangles[i].c.y < top) top = new_scene->triangles[i].c.y;
//        if(new_scene->triangles[i].c.y > bottom) bottom = new_scene->triangles[i].c.y;
//
//        new_scene->triangles[i].bb = (rect){f2(left, top), f2(right, bottom)};
//        new_scene->triangles[i].velocity = float2_rand_d(-0.5f, 0.5f, -0.5f, 0.5f);
//        new_scene->triangle_colors[i] = (i_rand_d(0x888888, 0xffffff) << 8) | 0xff;
//    }
//
//    new_scene->pixels = malloc(sizeof(uint32_t) * R_PIXEL_COUNT);

    return new_scene;
}

void R_render_scene(app_state_s* state_ptr, scene* scene_to_render) {
    assert(scene_to_render->actor_tree != nullptr, "Tried to render null scene!", return);
    mutex_locked_code(Engine_get_actor_mutex(), {
        Actor_render(scene_to_render->actor_tree, state_ptr);
    });
}