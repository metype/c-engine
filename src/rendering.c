#include "rendering.h"
#include "math.h"
#include "log.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

SDL_Texture *game_surface_0 = nullptr, *game_surface_1 = nullptr, *game_surface_2 = nullptr, *game_surface_3 = nullptr;
SDL_Texture *ui_surface_0 = nullptr, *ui_surface_1 = nullptr, *ui_surface_2 = nullptr;

uint8_t layers_in_use = 0;

SDL_Texture *canvas_3d = nullptr;

const int width = 640;
const int height = 360;
const int pixel_arr_length = width * height * 4;

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
        canvas_3d = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(canvas_3d, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(canvas_3d, SDL_SCALEMODE_NEAREST);
    }
    srandom(time(nullptr));

    const int triangle_count = 25;
    scene* new_scene = malloc(sizeof(scene));
    new_scene->triangle_count = triangle_count;
    new_scene->triangles = malloc(sizeof(tri) * triangle_count);
    new_scene->triangle_colors = malloc(sizeof(uint32_t) * triangle_count);

    float2_s half_size = (float2_s){350, 200};

    for(int i = 0; i < triangle_count; i++) {
        new_scene->triangles[i].a = float2_add(float2_mul_f(float2_rand_d(-200, 200, -200, 200), 0.3f), half_size);
        new_scene->triangles[i].b = float2_add(float2_mul_f(float2_rand_d(-200, 200, -200, 200), 0.3f), half_size);
        new_scene->triangles[i].c = float2_add(float2_mul_f(float2_rand_d(-200, 200, -200, 200), 0.3f), half_size);

        float left = 2000000, top = 2000000, right = -2000000, bottom = -2000000;
        if(new_scene->triangles[i].a.x < left) left = new_scene->triangles[i].a.x;
        if(new_scene->triangles[i].a.x > right) right = new_scene->triangles[i].a.x;
        if(new_scene->triangles[i].a.y < top) top = new_scene->triangles[i].a.y;
        if(new_scene->triangles[i].a.y > bottom) bottom = new_scene->triangles[i].a.y;

        if(new_scene->triangles[i].b.x < left) left = new_scene->triangles[i].b.x;
        if(new_scene->triangles[i].b.x > right) right = new_scene->triangles[i].b.x;
        if(new_scene->triangles[i].b.y < top) top = new_scene->triangles[i].b.y;
        if(new_scene->triangles[i].b.y > bottom) bottom = new_scene->triangles[i].b.y;

        if(new_scene->triangles[i].c.x < left) left = new_scene->triangles[i].c.x;
        if(new_scene->triangles[i].c.x > right) right = new_scene->triangles[i].c.x;
        if(new_scene->triangles[i].c.y < top) top = new_scene->triangles[i].c.y;
        if(new_scene->triangles[i].c.y > bottom) bottom = new_scene->triangles[i].c.y;

        new_scene->triangles[i].bb = (rect){f2(left, top), f2(right, bottom)};
        new_scene->triangles[i].velocity = float2_rand_d(-0.5f, 0.5f, -0.5f, 0.5f);
        new_scene->triangle_colors[i] = (i_rand_d(0x888888, 0xffffff) << 8) | 0xff;
    }

    new_scene->pixels = malloc(sizeof(uint32_t) * pixel_arr_length);

    return new_scene;
}

void R_render_scene(SDL_Renderer* renderer, scene* scene_to_render) {
    register int flag = 0;
    const int local_width = width;
    for(register int x = 0; x < width; x++) {
        for(register int y = 0; y < height; y++) {
            register int idx = x + (y * local_width);
            float2_s pt = f2((float)x, (float)y);
            for(register int i = 0; i < scene_to_render->triangle_count; i++) {
                if(!point_in_rect(scene_to_render->triangles[i].bb.tl, scene_to_render->triangles[i].bb.br, pt)) continue;
                if(!point_in_triangle(scene_to_render->triangles[i].a, scene_to_render->triangles[i].b, scene_to_render->triangles[i].c, pt, nullptr)) continue;

                flag = 1;
                scene_to_render->pixels[idx] = scene_to_render->triangle_colors[i];
                break;
            }

            if(!flag) scene_to_render->pixels[idx] = 0x000000FF;
            flag = 0;
        }
    }

    for(register int i = 0; i < scene_to_render->triangle_count; i++) {
        scene_to_render->triangles[i].a = float2_add(scene_to_render->triangles[i].a, scene_to_render->triangles[i].velocity);
        scene_to_render->triangles[i].b = float2_add(scene_to_render->triangles[i].b, scene_to_render->triangles[i].velocity);
        scene_to_render->triangles[i].c = float2_add(scene_to_render->triangles[i].c, scene_to_render->triangles[i].velocity);
        scene_to_render->triangles[i].bb.tl = float2_add(scene_to_render->triangles[i].bb.tl, scene_to_render->triangles[i].velocity);
        scene_to_render->triangles[i].bb.br = float2_add(scene_to_render->triangles[i].bb.br, scene_to_render->triangles[i].velocity);

        if(scene_to_render->triangles[i].bb.tl.x < 0 || scene_to_render->triangles[i].bb.br.x > (float)width) scene_to_render->triangles[i].velocity.x *= -1;
        if(scene_to_render->triangles[i].bb.tl.y < 0 || scene_to_render->triangles[i].bb.br.y > (float)height) scene_to_render->triangles[i].velocity.y *= -1;
    }

    uint32_t* pixels;
    int pitch = 0;
    SDL_LockTexture( canvas_3d, nullptr, (void**) &pixels, &pitch );
    memcpy(pixels, scene_to_render->pixels, pixel_arr_length);
    SDL_UnlockTexture( canvas_3d );
    SDL_RenderTexture(renderer, canvas_3d, nullptr, nullptr);
}