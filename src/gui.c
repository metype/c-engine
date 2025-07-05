#include <malloc.h>
#include "gui.h"
#include "log.h"
#include "definitions.h"
#include "viewport.h"
#include <SDL3/SDL.h>

#define MAX_GUI_OBJECTS 64

gui_object* objects = nullptr;
viewport* gui_vp = nullptr;

SDL_Color button_colors[] = {{.r = 210, .g = 210, .b = 210, .a = SDL_ALPHA_OPAQUE}, {.r = 150, .g = 195, .b = 239, .a = SDL_ALPHA_OPAQUE}, {.r = 100, .g = 100, .b = 100, .a = SDL_ALPHA_OPAQUE}, {.r = 240, .g = 240, .b = 240, .a = SDL_ALPHA_OPAQUE}, {.r = 20, .g = 20, .b = 20, .a = SDL_ALPHA_OPAQUE}};

void Gui_object_add(gui_object obj) {
    obj.list_end = false;
    if(!objects) {
        objects = malloc(sizeof(gui_object) * MAX_GUI_OBJECTS);
        for(int i = 0; i < MAX_GUI_OBJECTS; i++) {
            objects[i].list_end = true;
        }
    }
    for(int i = 0; i < MAX_GUI_OBJECTS; i++) {
        if(objects[i].list_end) {
            objects[i] = obj;
            if(i < MAX_GUI_OBJECTS - 1) {
                objects[i+1].list_end = true;
            }
            return;
        }
    }
    Log_print(LOG_LEVEL_ERROR, "Max GUI objects in one frame exceeded, cannot handle more than " STR(MAX_GUI_OBJECTS) " objects.");
}

bool Gui_button(int x, int y, int width, int height, const char* text) {
    if(width < 0 || height < 0) {
        width = ((int)strlen(text) + 1) * 8;
        height = 16;
    }

    float mx, my;
    SDL_MouseButtonFlags flags = SDL_GetMouseState(&mx, &my);
    bool button_hovered = (mx > (float)x && my > (float)y && mx < (float)x + width && my < (float)y + height);
    bool button_pressed = button_hovered && (flags & SDL_BUTTON_LEFT);

    Gui_object_add((gui_object){.type = GUI_BUTTON, .button = (gui_button) {.x = x, .y = y, .width = width, .height = height, .text = text, .pressed = button_pressed, .hovered = button_hovered}});

    return button_pressed;
}

void Gui_render(SDL_Renderer* renderer) {
    if(!objects) return;
    if(objects[0].list_end) return;

    if(!gui_vp) {
        gui_vp = malloc(sizeof(viewport));
        gui_vp->type = VIEWPORT_NO_SCALE;
        gui_vp->width = 1280;
        gui_vp->height = 720;
        gui_vp->x = 0;
        gui_vp->y = 0;
        Viewport_init(renderer, gui_vp);
    }

    Viewport_use(gui_vp);

    for(int i = 0; i < MAX_GUI_OBJECTS; i++) {
        if(objects[i].list_end) break;
        switch(objects[i].type) {
            case GUI_BUTTON:
            {
                gui_button btn = objects[i].button;
                SDL_Color active_color = btn.pressed ? button_colors[2] : btn.hovered ? button_colors[1] : button_colors[0];
                SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, active_color.a);
                SDL_FRect button_rect = {
                        .x = btn.x,
                        .y = btn.y,
                        .w = btn.width,
                        .h = btn.height,
                };
                SDL_RenderFillRect(renderer, &button_rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                float posx = (float)btn.x + (btn.pressed ? 5.f : 4.f);
                float posy = (float)btn.y + (btn.pressed ? 5.f : 4.f);

                SDL_RenderDebugText(renderer, posx, posy, btn.text);

                SDL_Color bottom_border = btn.pressed ? button_colors[3] : button_colors[4];
                SDL_Color top_border = btn.pressed ? button_colors[4] : button_colors[3];

                SDL_SetRenderDrawColor(renderer, top_border.r, top_border.g, top_border.b, SDL_ALPHA_OPAQUE);
                SDL_RenderLine(renderer, btn.x, btn.y, btn.x + btn.width, btn.y);
                SDL_RenderLine(renderer, btn.x, btn.y, btn.x, btn.y + btn.height);

                SDL_SetRenderDrawColor(renderer, bottom_border.r, bottom_border.g, bottom_border.b, SDL_ALPHA_OPAQUE);
                SDL_RenderLine(renderer, btn.x, btn.y + btn.height, btn.x + btn.width, btn.y + btn.height);
                SDL_RenderLine(renderer, btn.x + btn.width, btn.y, btn.x + btn.width, btn.y + btn.height);
            }
        }
    }

    Viewport_finish();

    objects[0].list_end = true;
}