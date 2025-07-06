#include <malloc.h>
#include "input.h"
#include "engine.h"

input_state_s* active_input_state = nullptr;

input_state_s* Input_get_state() {
    return active_input_state;
}

void Input_new_state() {
    if(active_input_state) free(active_input_state);
    active_input_state = malloc(sizeof(input_state_s));
    active_input_state->current_flags = 0;
    active_input_state->flags_last_frame = 0;
    active_input_state->flags_last_tick = 0;
}

void Input_event(SDL_Event* event) {
    switch(event->type) {
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            active_input_state->current_flags = SDL_GetMouseState(nullptr, nullptr);
            break;
    }
}


bool Input_mouse_button_pressed(SDL_MouseButtonFlags flags) {
    return (active_input_state->current_flags & flags) > 0;
}

bool Input_mouse_button_just_pressed(SDL_MouseButtonFlags flags) {
    thread_info_s* this_thread = Engine_get_thread_info(pthread_self());
    if(!this_thread || this_thread->type == THREAD_TYPE_RENDER) {
        // Render thread
        return (active_input_state->current_flags & flags) > 0 && (active_input_state->flags_last_frame & flags) == 0;
    } else {
        // Ticking thread
        return (active_input_state->current_flags & flags) > 0 && (active_input_state->flags_last_tick & flags) == 0;
    }
}

void Input_end_tick() {
    active_input_state->flags_last_tick = active_input_state->current_flags;
}

void Input_end_frame() {
    active_input_state->flags_last_frame = active_input_state->current_flags;
}