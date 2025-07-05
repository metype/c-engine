#include <stdlib.h>
#include <time.h>
#include "tilemap_actor.h"
#include "../util.h"
#include "../log.h"
#include "../viewport.h"

void tilemap_actor_init(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_INIT(self, tilemap_actor_data_s);

    srandom(time(NULL));

    data->tilemap = malloc(sizeof(long) * data->tilemap_width);

    data->up = false;
    data->left = false;
    data->right = false;
    data->down = false;

    for(int x = 0; x < data->tilemap_width; x++) {
        data->tilemap[x] = malloc(sizeof(long) * data->tilemap_height);
        for(int y = 0; y < data->tilemap_height; y++) {
            data->tilemap[x][y] = ((((x*y << 7) + y*y+x) << 7) + x*x+y) % (1 << 24);
        }
    }

    SDL_GetWindowSize(state_ptr->window_ptr, &data->sWidth, &data->sHeight);
}

void tilemap_actor_think(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_THINK(self, tilemap_actor_data_s);

    self->transform.position = (float2_s){.x = self->transform.position.x + (data->left ? -10.f : data->right ? 10.f : 0.f), .y = self->transform.position.y + (data->up ? -10.f : data->down ? 10.f : 0.f)};
}

void tilemap_actor_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, tilemap_actor_data_s);

    if(!data->tilemap) return;

    transform_s tr = Actor_get_transform_lerp(self, state_ptr->perf_metrics_ptr->time_in_tick);

    Viewport_get_active_size(&data->sWidth, &data->sHeight);

    const int xScreenMin = 0;
    const int yScreenMin = 0;
    const int xScreenMax = xScreenMin + data->sWidth;
    const int yScreenMax = yScreenMin + data->sHeight;

    float width = 16.f * tr.scale.x;
    float height = 16.f * tr.scale.y;

    /* If the top left (tl) of the tilemap is past the tl of the screen in
     * either axis, cut off the first rows/columns that are behind the tl.
     *
     * Speeds up the loops a bit, and requires less math be done and such and such
     *
     * - Emi
     */

    register int xMin = Max(0, (xScreenMin - tr.position.x) / width);
    register int yMin = Max(0, (yScreenMin - tr.position.y) / height);

    register int xMax = data->tilemap_width;
    register int yMax = data->tilemap_height;

    SDL_Renderer* renderer = state_ptr->renderer_ptr;

    for(register int x = xMin; x < xMax; x++) {
        if(x < xMin) continue;
        float xPos = ((float) x) * width;
        xPos += tr.position.x;

        if(xPos > xScreenMax) {
            xMax = x;
            continue;
        }

        if(xPos + width < xScreenMin) {
            xMin = x;
            continue;
        }

        for(register int y = yMin; y < yMax; y++) {

            float yPos = ((float) y) * height;

            yPos += tr.position.y;

            if(yPos > yScreenMax) {
                yMax = y;
                break;
            }

            if(yPos + width < yScreenMin) {
                yMin = y;
                continue;
            }

            long clr = data->tilemap[x][y];

            int r = (int)(clr >> 16) & 255;
            int g = (int)(clr >> 8) & 255;
            int b = (int)(clr) & 255;


            SDL_FRect rect = {.x = xPos, .y = yPos, .w = width, .h = height};
            SDL_SetRenderDrawColor(renderer, r, g, b, 120);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

//    SDL_SetRenderClipRect(state_ptr->renderer_ptr, nullptr);
}

void tilemap_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event) {
    ACTOR_PRE_EVENT(self, tilemap_actor_data_s);
    switch(event->type) {
        case SDL_EVENT_WINDOW_RESIZED:
            SDL_GetWindowSize(state_ptr->window_ptr, &data->sWidth, &data->sHeight);
            break;
    }
}

void tilemap_actor_recalc_bb(actor_s* self) {
    ACTOR_DATA_PTR(self, tilemap_actor_data_s);

    transform_s tr = Actor_get_transform(self);

    if(self->bb) free(self->bb);
    self->bb = malloc(sizeof(rect));

    float width = data->tilemap_width * 16 * tr.scale.x;
    float height = data->tilemap_height * 16 * tr.scale.y;

    self->bb->tl = (float2_s) {.x = self->transform.position.x, .y = self->transform.position.y};
    self->bb->br = (float2_s) {.x = self->bb->tl.x + width, .y = self->bb->tl.y + height};
}

string* tilemap_actor_serialize(void* serialized_obj) {
    if(!serialized_obj) return s("nil");
    tilemap_actor_data_s* data = serialized_obj;
    string* ret_str = s("");

    generic_serialize_value(data, tilemap_width, int, "width");
    generic_serialize_value(data, tilemap_height, int, "height");

    return ret_str;
}

void* tilemap_actor_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    tilemap_actor_data_s* data = malloc(sizeof(tilemap_actor_data_s));
    data->tilemap_height = 0;
    data->tilemap_width = 0;

    generic_deserialize_begin("tilemap")
        deserialize_stage_0()

        deserialize_stage_1();

        deserialize_stage_2({
            generic_deserialize_value(data, tilemap_width, int, width);
            generic_deserialize_value(data, tilemap_height, int, height);
        });
    generic_deserialize_end()

    return data;
}