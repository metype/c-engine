#include "script.h"
#include "lua_script.h"
#include "../log.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sys/stat.h>
#include "actor.h"
#include "lj_state.h"
#include "../structures/stack.h"

enum custom_lua_event_code {
    CUSTOM_EVENT_UNKNOWN = 0,
    CUSTOM_EVENT_KEY = 1,
    CUSTOM_EVENT_MOUSE = 2,
    CUSTOM_EVENT_WINDOW_RESIZE = 3,
    CUSTOM_EVENT_WINDOW_MAXIMIZE = 4,
    CUSTOM_EVENT_WINDOW_MINIMIZE = 5,
    CUSTOM_EVENT_FOCUS_LOST = 6,
    CUSTOM_EVENT_FOCUS_GAINED = 7,
};

stack* allocated_ptrs = nullptr;

void Lua_create_stack_table(lua_State* state, lua_table_entry table_data[], int len) { // NOLINT(*-no-recursion)
    lua_newtable(state);
    for(int i = 0; i < len; i++) {
        int type = table_data[i].type;

        switch(type) {
            case LUA_TBOOLEAN:
                lua_pushboolean(state, table_data[i].boolean_val);
                break;
            case LUA_TTABLE:
                Lua_create_stack_table(state, table_data[i].child_table, table_data[i].len);
                break;
            case LUA_TSTRING:
                lua_pushstring(state, table_data[i].string_val);
                break;
            case LUA_TFLOAT:
                lua_pushnumber(state, table_data[i].float_val);
                break;
            case LUA_TINTEGER:
                lua_pushinteger(state, table_data[i].integer_val);
                break;
            default:
                break;
        }

        lua_setfield(state, -2, table_data[i].key);
    }
}

bool Lua_script_load_internal(const char* script_path, script* script) {
    lua_script* Lscript = malloc(sizeof(lua_script));
    Lscript->state = luaL_newstate();
    script->script_path = strdup(script_path);
    Lscript->finished_setup = false;

    if(!allocated_ptrs) {
        allocated_ptrs = Stack_init(64);
    }

    struct stat file_stat;
    int err = stat(script_path, &file_stat);
    if (err != 0) {
        return false;
    }
    Lscript->modified_time = file_stat.st_mtime;

    // fuck it we give em everything, to be frank this is kinda what we want
    // since this engine is designed to run a lotta Lua lol
    luaL_openlibs(Lscript->state);
    int status = luaL_loadfile(Lscript->state, script_path);

    if(status == LUA_ERRSYNTAX) {
        Log_printf(LOG_LEVEL_ERROR, "Error parsing Lua script \"%s\"!! Syntax error!", script_path);
        return false;
    }
    if(status != LUA_OK) {
        Log_printf(LOG_LEVEL_ERROR, "Error parsing Lua script \"%s\"!! Unknown error!", script_path);
        return false;
    }

    int ret = lua_pcall(Lscript->state, 0, 0, 0); // tell Lua to run the Lscript

    if (ret != 0) {
        Log_printf(LOG_LEVEL_ERROR, "Error parsing Lua script \"%s\"!! %s", script_path, lua_tostring(Lscript->state, -1));
        return false;
    }
    script->script_ptr = Lscript;
    return true;
}

script* Lua_script_load(const char* script_path) {
    script* script = malloc(sizeof(struct script_s));
    script->setup_func = &Lua_script_setup;
    script->event_func = &Lua_script_event;
    script->init_func = &Lua_script_init;
    script->render_func = &Lua_script_render;
    script->think_func = &Lua_script_think;

    if(Lua_script_load_internal(script_path, script)) return script;

    return nullptr;
}
#define custom_event_add(event) lua_pushinteger(Lscript->state, CONCAT(CUSTOM_,event)); lua_setglobal(Lscript->state, #event);
#define cfunc_add(func_ptr, name) lua_pushcfunction(Lscript->state, func_ptr); lua_setglobal(Lscript->state, name);

void Lua_script_setup(script* script) {
    lua_script* Lscript = script->script_ptr;

    // Overwrite print()
    cfunc_add(&Lua_print, "print");

    // Add actor functions
    cfunc_add(&Lua_set_actor_data, "SetActorData");
    cfunc_add(&Lua_get_actor_handle_from_path, "GetActorHandle");
    cfunc_add(&Lua_get_actor_from_path, "GetActor");
    cfunc_add(&Lua_get_actor_path, "GetActorPath");

    custom_event_add(EVENT_UNKNOWN);
    custom_event_add(EVENT_KEY);
    custom_event_add(EVENT_MOUSE);
    custom_event_add(EVENT_WINDOW_RESIZE);
    custom_event_add(EVENT_WINDOW_MINIMIZE);
    custom_event_add(EVENT_WINDOW_MAXIMIZE);
    custom_event_add(EVENT_FOCUS_GAINED);
    custom_event_add(EVENT_FOCUS_LOST);

    Lscript->finished_setup = true;
}

#define lua_args_start(argcount) { int argc = argcount;
#define test_lua_arg(argpos, luatype) if(!CONCAT(lua_is,luatype)(state, 0 - (argc - argpos))) return 0;
#define def_lua_arg(val, argpos, luatype) val = CONCAT(lua_to,luatype)(state, 0 - (argc - argpos));
#define lua_args_end() }

#define lua_from_table(name, key, type, luatype, idx) lua_getfield(state, idx, key); type name = CONCAT(lua_to,luatype)(state, -1); lua_pop(state, 1);
#define lua_from_table_with_check(toset, key, luatype, idx) lua_getfield(state, idx, key); if(!lua_isnil(state, -1)) { toset = CONCAT(lua_to,luatype)(state, -1); } lua_pop(state, 1);
#define bool_from_table(key, toset) lua_from_table_with_check(toset, key, boolean, -1);
#define int_from_table(key, toset) lua_from_table_with_check(toset, key, integer, -1);
#define float_from_table(key, toset) lua_from_table_with_check(toset, key, number, -1);

int Lua_get_actor_path(lua_State* state) {
    int handle = 0;

    lua_args_start(1);
        test_lua_arg(0, number);
        def_lua_arg(handle, 0, integer);
    lua_args_end();

    actor_s* actor = Actor_from_handle(handle);

    if(!actor) {
        lua_pushstring(state, "");
        return 1;
    }

    char* path = Actor_get_path(actor);
    char* final_path = malloc(strlen(path) * sizeof(char) + 1);

    Stack_push(allocated_ptrs, path);
    Stack_push(allocated_ptrs, final_path);

    final_path[0] = '/';

    for(int i = 1; i <= strlen(path); i++) {
        final_path[i] = path[i-1];
    }

    lua_pushstring(state, final_path);
    return 1;
}

int Lua_get_actor_handle_from_path(lua_State* state) {

    if(!lua_isnumber(state, -2) && !lua_isstring(state, -1)) return 0;
    int handle = -1;
    if(lua_isnumber(state, -2)) handle = (int) lua_tointeger(state, -2);
    const char* path = lua_tostring(state, -1);

    if(!path) {
        lua_pushinteger(state, -1);
        return 1;
    }

    actor_s* actor;

    if(handle == -1) {
        actor = Actor_from_path(path);
    } else {
        actor = Actor_from_path_rel(Actor_from_handle(handle), path);
    }

    if(actor) {
        lua_pushinteger(state, actor->handle);
    } else {
        lua_pushinteger(state, -1);
    }

    return 1;
}

int Lua_get_actor_from_path(lua_State* state) {
    if(!lua_isnumber(state, -1) && !lua_isstring(state, -1)) return 0;

    if(lua_isnumber(state, -1)) {
        int handle = (int) lua_tointeger(state, -1);
        Lua_script_setup_actor_table(state, Actor_from_handle(handle));
        return 1;
    }

    if(lua_isstring(state, -1)) {
        const char* path = lua_tostring(state, -1);
        Lua_script_setup_actor_table(state, Actor_from_path(path));
        return 1;
    }

    // should be actively impossible
    lua_pushnil(state);
    return 1;
}

int Lua_set_actor_data(lua_State* state) {
    int handle = 0;

    lua_args_start(2);
        test_lua_arg(0, number);
        test_lua_arg(1, table);
        def_lua_arg(handle, 0, integer);
    lua_args_end();

    actor_s* actor = Actor_from_handle(handle);

    if(actor) {
        bool_from_table("visible", actor->visible);

        lua_getfield(state, -1, "transform");
        float_from_table("x", actor->transform.position.x);
        float_from_table("y", actor->transform.position.y);
        float_from_table("scale_x", actor->transform.scale.x);
        float_from_table("scale_y", actor->transform.scale.y);
        float_from_table("rotation", actor->transform.rotation);
        lua_pop(state, 1);
    }

    return 0;
}

int Lua_print(lua_State* state) {
    if(!lua_isnumber(state, -2) && !lua_isstring(state, -1)) return -1;
    int lvl = LOG_LEVEL_INFO;
    if(lua_isnumber(state, -2) && lua_tointeger(state, -2) != 0) lvl = (int) lua_tointeger(state, -2);
    const char* str = lua_tostring(state, -1);
    Log_print(lvl, str);
    return 0;
}

void Lua_script_init(script* script, actor_s* actor) {
    lua_script* Lscript = script->script_ptr;
    script->parent = actor;

    lua_getfield(Lscript->state, LUA_GLOBALSINDEX, "_init");
    if(lua_isnil(Lscript->state, -1)) {
        lua_pop(Lscript->state, 1);
        return;
    }

    Lua_script_setup_actor_table(Lscript->state, actor);
    lua_call(Lscript->state, 1, 0);
}

void Lua_script_render(script* script, actor_s* actor, app_state_s* state) {
    lua_script* Lscript = script->script_ptr;

    lua_getfield(Lscript->state, LUA_GLOBALSINDEX, "_render");
    if(lua_isnil(Lscript->state, -1)) {
        lua_pop(Lscript->state, 1);
        return;
    }

    Lua_script_setup_actor_table(Lscript->state, actor);
    lua_pushnumber(Lscript->state, state->perf_metrics_ptr->dt);
    lua_call(Lscript->state, 2, 0);
}

void Lua_script_think(script* script, actor_s* actor, app_state_s* state) {
    lua_script* Lscript = script->script_ptr;

    lua_getfield(Lscript->state, LUA_GLOBALSINDEX, "_think");
    if(lua_isnil(Lscript->state, -1)) {
        lua_pop(Lscript->state, 1);
        return;
    }

    Lua_script_setup_actor_table(Lscript->state, actor);
    lua_call(Lscript->state, 1, 0);

    void* val;
    while( (val = Stack_pull(allocated_ptrs)) ) {
        free(val);
    }
}

void Lua_script_event(script* script, actor_s* actor, app_state_s* state, SDL_Event* event) {
    lua_script* Lscript = script->script_ptr;
    if(!Lscript->finished_setup) return;

    lua_getfield(Lscript->state, LUA_GLOBALSINDEX, "_event");
    if(lua_isnil(Lscript->state, -1)) {
        lua_pop(Lscript->state, 1);
        return;
    }

    Lua_script_setup_actor_table(Lscript->state, actor);

    int event_type;

    switch(event->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            lua_table_entry key_table[] = {
                    {.key="code", .integer_val = event->key.scancode, .type = LUA_TINTEGER},
                    {.key="down", .boolean_val = event->key.down, .type = LUA_TBOOLEAN},
                    {.key="repeat", .boolean_val = event->key.repeat, .type = LUA_TBOOLEAN},
            };
            lua_table_entry event_table[] = {
                    {.key = "type", .integer_val = CUSTOM_EVENT_KEY, .type = LUA_TINTEGER},
                    {.key = "key", .child_table = key_table, .type = LUA_TTABLE, .len = 3},
            };
            Lua_create_stack_table(Lscript->state, event_table, 2);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION:
        {
            lua_table_entry mouse_table[] = {
                    {.key="x", .float_val = event->motion.x, .type = LUA_TFLOAT},
                    {.key="y", .float_val = event->motion.y, .type = LUA_TFLOAT},
                    {.key="xrel", .float_val = event->motion.xrel, .type = LUA_TFLOAT},
                    {.key="yrel", .float_val = event->motion.yrel, .type = LUA_TFLOAT},
                    {.key="button", .integer_val = -1, .type = LUA_TINTEGER},
                    {.key="down", .boolean_val = 0, .type = LUA_TBOOLEAN},
            };
            lua_table_entry event_table[] = {
                    {.key = "type", .integer_val = CUSTOM_EVENT_MOUSE, .type = LUA_TINTEGER},
                    {.key = "mouse", .child_table = mouse_table, .type = LUA_TTABLE, .len = 6},
            };
            Lua_create_stack_table(Lscript->state, event_table, 2);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            lua_table_entry mouse_table[] = {
                    {.key="x", .float_val = event->button.x, .type = LUA_TFLOAT},
                    {.key="y", .float_val = event->button.y, .type = LUA_TFLOAT},
                    {.key="xrel", .float_val = 0, .type = LUA_TFLOAT},
                    {.key="yrel", .float_val = 0, .type = LUA_TFLOAT},
                    {.key="button", .integer_val = event->button.button, .type = LUA_TINTEGER},
                    {.key="down", .boolean_val = event->button.down, .type = LUA_TBOOLEAN},
            };
            lua_table_entry event_table[] = {
                    {.key = "type", .integer_val = CUSTOM_EVENT_MOUSE, .type = LUA_TINTEGER},
                    {.key = "mouse", .child_table = mouse_table, .type = LUA_TTABLE, .len = 6},
            };
            Lua_create_stack_table(Lscript->state, event_table, 2);
            break;
        }
        case SDL_EVENT_WINDOW_RESIZED:
            event_type = CUSTOM_EVENT_WINDOW_RESIZE;
            goto window_event;
        case SDL_EVENT_WINDOW_MAXIMIZED:
            event_type = CUSTOM_EVENT_WINDOW_MAXIMIZE;
            goto window_event;
        case SDL_EVENT_WINDOW_MINIMIZED:
            event_type = CUSTOM_EVENT_WINDOW_MINIMIZE;
            goto window_event;
        window_event:
        {
            int width;
            int height;
            SDL_GetWindowSize(state->window_ptr, &width, &height);

            lua_table_entry window_table[] = {
                    {.key="width", .integer_val = width, .type = LUA_TINTEGER},
                    {.key="height", .integer_val = height, .type = LUA_TINTEGER},
            };
            lua_table_entry event_table[] = {
                    {.key = "type", .integer_val = event_type, .type = LUA_TINTEGER},
                    {.key = "window", .child_table = window_table, .type = LUA_TTABLE, .len = 2},
            };
            Lua_create_stack_table(Lscript->state, event_table, 2);
            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
            lua_table_entry event_table[] = {
                    {.key = "type", .integer_val = CUSTOM_EVENT_FOCUS_LOST, .type = LUA_TINTEGER},
            };
            Lua_create_stack_table(Lscript->state, event_table, 1);
            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            lua_table_entry event_table[] = {
                    {.key = "type", .integer_val = CUSTOM_EVENT_FOCUS_GAINED, .type = LUA_TINTEGER},
            };
            Lua_create_stack_table(Lscript->state, event_table, 1);

#if CENGINE_DEBUG
            // Check file modified time
            struct stat file_stat;
            int err = stat(script->script_path, &file_stat);
            if (err != 0) {
                return;
            }

            // If the modified time has updated, then the file has changed!
            if(Lscript->modified_time < file_stat.st_mtime) {

                // Go ahead and reload it! :D
                char* script_path = strdup(script->script_path);
                free(script->script_path);

                free(script->script_ptr);

                Lua_script_load_internal(script_path, script); // Load the script
                Lua_script_setup(script); // Run setup
                Lua_script_init(script, actor); // Run init again just to be safe? Not sure about this one tbh

                Log_printf(LOG_LEVEL_INFO, "File change detected! Reloaded script %s", script_path);

                free(script_path);
                return;
            }
#endif
        break;
        }
        default: {
            lua_table_entry event_table[] = {
                    {.key = "type", .float_val = CUSTOM_EVENT_UNKNOWN, .type = LUA_TINTEGER}
            };
            Lua_create_stack_table(Lscript->state, event_table, 1);
        }

    }

    lua_call(Lscript->state, 2, 0);
}

void Lua_script_setup_actor_table(lua_State* state, actor_s* actor) {
    if(!actor) {
        lua_pushnil(state);
        return;
    }

    lua_table_entry transform_table[] = {
            {.key="x", .type = LUA_TFLOAT, .float_val = actor->transform.position.x},
            {.key="y", .type = LUA_TFLOAT, .float_val = actor->transform.position.y},
            {.key="scale_x", .type = LUA_TFLOAT, .float_val = actor->transform.scale.x},
            {.key="scale_y", .type = LUA_TFLOAT, .float_val = actor->transform.scale.y},
            {.key="rotation", .type = LUA_TFLOAT, .float_val = actor->transform.rotation},
            {.key="top_level", .type = LUA_TBOOLEAN, .boolean_val = actor->transform.top_level},
    };

    lua_table_entry actor_table[] = {

            /* <Read only!> */
            {.key = "life", .type = LUA_TINTEGER, .integer_val = actor->ticks_since_spawn},
            {.key = "id", .type = LUA_TSTRING, .string_val = actor->actor_id},
            {.key = "name", .type = LUA_TSTRING, .string_val = actor->name},
            {.key = "handle", .type = LUA_TINTEGER, .integer_val = actor->handle},
            /* </Read only!> */

            {.key = "visible", .type = LUA_TBOOLEAN, .boolean_val = actor->visible},
            {.key = "transform", .child_table = transform_table, .type = LUA_TTABLE, .len = 6},
    };

    Lua_create_stack_table(state, actor_table, 6);
}