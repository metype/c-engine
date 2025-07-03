#include <malloc.h>
#include <stdlib.h>
#include "viewport_actor.h"
#include "../log.h"
#include "../util.h"
#include "../audio.h"
#include "../viewport.h"

void viewport_actor_init(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_INIT(self, viewport_actor_data_s);

    data->previous_vp = nullptr;
}

void viewport_actor_think(actor_s* self, app_state_s* state_ptr) {

}

void viewport_actor_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, viewport_actor_data_s);
    data->previous_vp = Viewport_active();
    Viewport_use(state_ptr->renderer_ptr, data->vp);
}

void viewport_actor_late_render(actor_s* self, app_state_s* state_ptr) {
    ACTOR_PRE_RENDER(self, viewport_actor_data_s);
    Viewport_use(state_ptr->renderer_ptr, data->previous_vp);
}

void viewport_actor_event(actor_s* self, app_state_s* state_ptr, SDL_Event* event) {

}

string* viewport_actor_serialize(void* serialized_obj) {
    if(!serialized_obj) return s("nil");
    viewport_actor_data_s* data = serialized_obj;
    string* ret_str = s("");

    generic_serialize_obj_ptr(data->vp, "viewport", vp);

    return ret_str;
}

void* viewport_actor_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    viewport_actor_data_s* data = malloc(sizeof(viewport_actor_data_s));
    data->vp = nullptr;

    const char *deserializer_name = "viewport";
    unsigned len = S_length(str);
    unsigned marcher = 0;
    unsigned buf_idx = 0;
    char buffer[1024];
    char name_buf[128];
    char value_buf[128];
    char parser_buf[128];
    unsigned stage = 0;
    bool used = false;
    while (marcher < len) {
        if (str->c_str[marcher] != ' ' && str->c_str[marcher] != '\n') {
            if (str->c_str[marcher] == '\t') {
                marcher++;
                continue;
            }
            if (buf_idx >= 1024) {
                Log_print(LOG_LEVEL_ERROR, "Inevitable buffer overrun detected! You sneaky bugger! Get truncated!");
                while (str->c_str[marcher] != ' ' && str->c_str[marcher] != '\n')marcher++;
            }
            else {
                buffer[buf_idx] = str->c_str[marcher];
                buf_idx++;
                marcher++;
                continue;
            }
        }
        buffer[buf_idx] = 0;
        buf_idx = 0;
        if (stage == 0) { snprintf(name_buf, 128, "%s", buffer); }

        if (stage == 1) {
            snprintf(value_buf, 128, "%s", buffer);
            do {
                if (strcmp(name_buf, "vp") == 0) {
                    if (strcmp(value_buf, "{") == 0) {
                        data->vp = (viewport *) Deserialize_block_to_obj(S_convert((str->c_str + marcher - 1)));
                        unsigned block_count = 1;
                        while (block_count) {
                            marcher++;
                            if (str->c_str[marcher] == '{')block_count++;
                            if (str->c_str[marcher] == '}')block_count--;
                            used = true;
                        }
                        while (str->c_str[marcher] != '\n')marcher++;
                        stage = -1;
                    }
                }
            }
            while (0);
        }

        if (stage == 2) {
            snprintf(parser_buf, 128, "%s", buffer);
            stage = -1;
            if (!used) {
                Log_printf(LOG_LEVEL_WARNING, "Key %s not used in object %s, but referenced in file?", name_buf,
                           deserializer_name);
                used = false;
            }
        }
        stage++;
        marcher++;
    }

    return data;
}