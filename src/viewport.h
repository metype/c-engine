#ifndef CENGINE_VIEWPORT_H
#define CENGINE_VIEWPORT_H

#include "string.h"

enum viewport_type {
    VIEWPORT_NO_SCALE = 0,
    VIEWPORT_SCALE = 1,
    VIEWPORT_SCALE_LINEAR = 2,
    VIEWPORT_SCALE_KEEP = 3,
    VIEWPORT_SCALE_KEEP_LINEAR = 4,
};

typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_FRect SDL_FRect;

typedef struct viewport_s {
    int type;
    int x;
    int y;
    int width;
    int height;
    SDL_Texture* texture;
    SDL_Renderer* render_target;
} viewport;

string* Viewport_serialize(void* serialized_obj);
void* Viewport_deserialize(string* str);

void Viewport_init(SDL_Renderer* renderer, viewport* vp);
void Viewport_use(viewport* vp);
void Viewport_finish();
viewport* Viewport_active();
void Viewport_get_active_size(int* width, int* height);

SDL_Renderer* Active_renderer();

//void Viewport_finish(SDL_Renderer* renderer, viewport* vp);
#endif //CENGINE_VIEWPORT_H
