#ifndef CENGINE_GUI_H
#define CENGINE_GUI_H
typedef struct SDL_Renderer SDL_Renderer;

enum gui_obj_type {
    GUI_BUTTON = 0,
};

typedef struct gui_button_s {
    int x;
    int y;
    int width;
    int height;
    bool hovered;
    bool pressed;
    bool assume_size;
    const char* text;
} gui_button;

typedef struct gui_object_s {
    bool list_end;
    int type;
    union {
        gui_button button;
    };
} gui_object;

void Gui_object_add(gui_object obj);
bool Gui_button(int x, int y, int width, int height, const char* text);
void Gui_render(SDL_Renderer* renderer);
#endif //CENGINE_GUI_H
