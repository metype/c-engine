#ifndef CENGINE_SCRIPT_H
#define CENGINE_SCRIPT_H

#include "../callbacks.h"
#include "SDL3/SDL_events.h"

typedef struct script_s script;
typedef struct actor_s actor_s;
typedef struct app_state app_state_s;

typedef struct script_s {
    void* script_ptr;
    char* script_path;
    actor_s* parent;

    M_CALLBACK(setup_func, void, struct script_s*);

    M_CALLBACK(render_func, void, script*, actor_s*, app_state_s* state);
    M_CALLBACK(think_func, void, script*, actor_s*, app_state_s* state);
    M_CALLBACK(event_func, void, script*, actor_s*, app_state_s* state, SDL_Event* event);
    M_CALLBACK(init_func, void, script*, actor_s*);
} script;

void Script_setup(script* script);
#endif //CENGINE_SCRIPT_H
