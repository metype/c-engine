#ifndef CENGINE_RENDERING_H
#define CENGINE_RENDERING_H
#include <stdint.h>
#include "math.h"

typedef enum screen_layer : uint8_t {
    LAYER_BASE   = 0b00000001,
    LAYER_GAME_0 = 0b00000010,
    LAYER_GAME_1 = 0b00000100,
    LAYER_GAME_2 = 0b00001000,
    LAYER_GAME_3 = 0b00010000,
    LAYER_UI_0   = 0b00100000,
    LAYER_UI_1   = 0b01000000,
    LAYER_UI_2   = 0b10000000,
} screen_layer;

typedef struct rect {
    float2_s tl;
    float2_s br;
} rect;

typedef struct tri {
    float2_s a;
    float2_s b;
    float2_s c;
    float2_s velocity;
    rect bb;
} tri;

typedef struct scene {
    int triangle_count;
    tri* triangles;
    uint32_t* triangle_colors;
    uint32_t* pixels;
} scene;

typedef struct SDL_Renderer SDL_Renderer;

void R_switch_layer(SDL_Renderer* renderer, uint8_t layer);
void R_render_composite(SDL_Renderer* renderer);
scene* R_init_scene(SDL_Renderer* renderer);
void R_render_scene(SDL_Renderer* renderer, scene* scene_to_render);
#endif //CENGINE_RENDERING_H
