#ifndef CENGINE_LUA_SCRIPT_H
#define CENGINE_LUA_SCRIPT_H

#include <bits/types/time_t.h>
#include "SDL3/SDL_events.h"

typedef struct script_s script;
typedef struct actor_s actor_s;
typedef struct lua_State lua_State;
typedef struct app_state app_state_s;

typedef struct lua_script_s {
    lua_State* state;
    time_t modified_time;
    bool finished_setup;
} lua_script;

#define LUA_TINTEGER LUA_TNUMBER
#define LUA_TFLOAT 32

typedef struct lua_table_entry_s {
    union {
        bool boolean_val;
        float float_val;
        int integer_val;
        const char* string_val;
        struct lua_table_entry_s* child_table;
    };
    int type;
    const char* key;
    int len;
} lua_table_entry;

void Lua_script_init(script* script, actor_s* actor);
void Lua_script_render(script* script, actor_s* actor, app_state_s* state);
void Lua_script_think(script* script, actor_s* actor, app_state_s* state);
void Lua_script_event(script* script, actor_s* actor, app_state_s* state, SDL_Event* event);
void Lua_script_setup(script* script);
bool Lua_script_load_internal(const char* script_path, script* script);
script* Lua_script_load(const char* script_path);

void Lua_create_stack_table(lua_State* state, lua_table_entry table_data[], int len);
void Lua_script_setup_actor_table(lua_State* state, actor_s* actor);

int Lua_gui_button(lua_State* state);
int Lua_get_actor_path(struct lua_State*);
int Lua_get_actor_from_path(struct lua_State*);
int Lua_get_actor_handle_from_path(struct lua_State*);
int Lua_set_actor_data(struct lua_State*);
int Lua_print(struct lua_State*);
#endif //CENGINE_LUA_SCRIPT_H
