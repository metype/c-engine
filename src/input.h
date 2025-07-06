#ifndef CENGINE_INPUT_H
#define CENGINE_INPUT_H

#include <SDL3/SDL.h>

typedef struct input_state {
    SDL_MouseButtonFlags current_flags;
    SDL_MouseButtonFlags flags_last_frame;
    SDL_MouseButtonFlags flags_last_tick;
} input_state_s;

input_state_s* Input_get_state();
void Input_new_state();
void Input_event(SDL_Event* event);

bool Input_mouse_button_pressed(SDL_MouseButtonFlags flags);
bool Input_mouse_button_just_pressed(SDL_MouseButtonFlags flags);

void Input_end_tick();
void Input_end_frame();

#endif //CENGINE_INPUT_H
