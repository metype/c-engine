#ifndef CENGINE_RENDERING_H
#define CENGINE_RENDERING_H
#include <stdint.h>

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

typedef struct SDL_Renderer SDL_Renderer;

void R_switch_layer(SDL_Renderer* renderer, uint8_t layer);
void R_render_composite(SDL_Renderer* renderer);
#endif //CENGINE_RENDERING_H
