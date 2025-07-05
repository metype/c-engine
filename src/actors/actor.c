#include "actor.h"
#include "../log.h"
#include "../errors.h"
#include "../structures/hashmap.h"
#include "test_actor.h"
#include "tilemap_actor.h"
#include "../util.h"
#include <pthread.h>
#include <malloc.h>
#include "script.h"
#include "../structures/stack.h"
#include "lua_script.h"
#include "viewport_actor.h"
#include "editor_actor.h"

#if CENGINE_WIN32
#include "../win32_stdlib.h"
#endif

void empty_init(actor_s* actor, app_state_s* state_ptr) {}
void empty_think(actor_s* actor, app_state_s* state_ptr) {}
void empty_render(actor_s* actor, app_state_s* state_ptr) {}
void empty_event(actor_s* actor, app_state_s* state_ptr, SDL_Event* event) {}
void empty_recalc_bb(actor_s* actor) {actor->bb = malloc(sizeof(rect)); *actor->bb = (rect) {.tl = {.x = 0, .y = 0}, .br = {.x = 0, .y = 0}}; }

string* empty_serialize(void* serialized_obj) { return so("nil"); }
void* empty_deserialize(string* str) { return nullptr; }

stack* actor_stack = nullptr;

hash_map_s* registered_actor_serialization_types = nullptr;
hash_map_s* registered_actor_deserialization_types = nullptr;

void Actor_tick(actor_s* actor, struct app_state* state_ptr) { // NOLINT(*-no-recursion)
    if(!actor) return;

    if(actor->ticks_since_spawn == 0) {
        // Do the check once and then assume from here out, these values do not change
        // This isn't necessarily true, but it should be for compliant Actors so
        // Fewer checks make code run faster :)
        if(!actor->init) actor->init = &empty_init;
        if(!actor->thinker) actor->thinker = &empty_think;
        if(!actor->render) actor->render = &empty_render;
        if(!actor->late_render) actor->late_render = &empty_render;

        actor->init(actor, state_ptr);
        if(actor->script) actor->script->setup_func(actor->script);
        if(actor->script) actor->script->init_func(actor->script, actor);
    }

    if(actor->process) {
        actor->thinker(actor, state_ptr);
        if(actor->script) actor->script->think_func(actor->script, actor, state_ptr);
    }

    for(register int i = 0; i < actor->children->element_count; i++) {
        actor_s* child = (actor_s*)actor->children->array[i];
        if(child->process && !actor->process) {
            child->process = false;
        }
        Actor_tick(child, state_ptr);
    }

    actor->ticks_since_spawn++;
}

void Actor_render(actor_s* actor, struct app_state* state_ptr) { // NOLINT(*-no-recursion)
    if(!actor) return;
    if(!actor->visible) return;

    actor->render(actor, state_ptr);
    if(actor->script) actor->script->render_func(actor->script, actor, state_ptr);

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_render(actor->children->array[i], state_ptr);
    }

    actor->late_render(actor, state_ptr);
}

void Actor_event(actor_s* actor, struct app_state* state_ptr, SDL_Event* event) { // NOLINT(*-no-recursion)
    if(!actor) return;

    actor->event(actor, state_ptr, event);
    if(actor->script) actor->script->event_func(actor->script, actor, state_ptr, event);

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_event(actor->children->array[i], state_ptr, event);
    }
}

void Actor_update_transforms(actor_s* actor) { // NOLINT(*-no-recursion)
    if(!actor) return;

    actor->pre_transform = actor->transform;

    for(register int i = 0; i < actor->children->element_count; i++) {
        Actor_update_transforms(actor->children->array[i]);
    }
}

void Actor_add_child(actor_s* parent, actor_s* child) {
    assert(parent != nullptr, "Parent cannot be null!", return);
    assert(child != nullptr, "Child to add cannot be null!", return);
    list_add(parent->children, child, return);
    child->parent = parent;
}

void Actor_add_sibling(actor_s* sibling, actor_s* new_sibling) {
    assert(sibling != nullptr, "Sibling cannot be null!", return);
    assert(sibling->parent != nullptr, "Trying to add a sibling to a actor that cannot have siblings (no parent).", return);
    Actor_add_child(sibling->parent, new_sibling);
    new_sibling->parent = sibling->parent;
}

void Actor_remove_child(actor_s* parent, actor_s* child) {
    assert(parent != nullptr, "Parent cannot be null!", return);
    assert(child != nullptr, "Child to remove cannot be null!", return);

    for(register int i = 0; i < parent->children->element_count; i++) {
        if(parent->children->array[i] == child) {
            parent->children->array[i] = nullptr;
            break;
        }
    }
    Actor_cleanup_children(parent);
    child->parent = nullptr;
}

void Actor_cleanup_children(actor_s* actor) {
    assert(actor != nullptr, "Parent cannot be null!", return);
    list_remove(actor->children, nullptr);
}

actor_s* Actor_create_s(transform_s transform, char* actor_id) { return Actor_create(transform, Actor_get_def(actor_id)); }

actor_s* Actor_create(transform_s transform, actor_def_s* actor_definition) {
    if(!actor_stack) {
        actor_stack = Stack_init(16384);
    }

    actor_s* new_actor = malloc(sizeof(actor_s));

    new_actor->actor_id = strdup(actor_definition->actor_id);
    new_actor->ticks_since_spawn = 0;
    new_actor->visible = true;
    new_actor->process = true;

    new_actor->parent = nullptr;
    new_actor->children = list_new(sizeof(actor_s*));

    new_actor->thinker = actor_definition->thinker;
    new_actor->init = actor_definition->init;
    new_actor->render = actor_definition->render;
    new_actor->late_render = actor_definition->late_render;
    new_actor->event = actor_definition->event;
    new_actor->recalc_bb = actor_definition->recalc_bb;

    new_actor->bb = nullptr;
    new_actor->transform = transform;

    new_actor->handle = Stack_push(actor_stack, new_actor);

    return new_actor;
}

actor_s* Actor_from_handle(int handle) {
    if(!actor_stack) return nullptr;

    return Stack_get(actor_stack, handle);
}

void Actor_destroy(actor_s* actor) {
    if(actor_stack) {
        Stack_set(actor_stack, actor->handle, nullptr);
    }
    free(actor->actor_id);
    free(actor->children->array);
    free(actor->children);
}

hash_map_s* actor_defs = nullptr;
pthread_mutex_t actor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t actor_mutex_attr;

actor_def_s* Actor_get_def(char* actor_id) {
    actor_def_s *def = nullptr;
    mutex_locked_code(&actor_mutex, {
        if (!actor_defs) return nullptr;
        def = map_get(actor_defs, actor_id, actor_def_s*);
    });
    return def;
}

void Actor_register_def(char* actor_id,
                        actor_init(actor_init),
                        actor_thinker(actor_think),
                        actor_render(actor_render),
                        actor_render(actor_late_render),
                        actor_event(actor_event),
                        actor_recalc_bb(actor_recalc_bb)) {
    if(!actor_id) return;

    if(!actor_init) actor_init = &empty_init;
    if(!actor_render) actor_render = &empty_render;
    if(!actor_think) actor_think = &empty_think;
    if(!actor_event) actor_event = &empty_event;

    actor_def_s* actor_def = malloc(sizeof(actor_def_s));

    actor_def->thinker = actor_think;
    actor_def->init = actor_init;
    actor_def->render = actor_render;
    actor_def->late_render = actor_late_render;
    actor_def->event = actor_event;
    actor_def->recalc_bb = actor_recalc_bb;

    actor_def->actor_id = strdup(actor_id);

    mutex_locked_code(&actor_mutex, {
        if (!actor_defs) {
            actor_defs = malloc(sizeof(hash_map_s));
            Map_init(actor_defs);
        }

        map_set(actor_defs, actor_id, actor_def);
    });

}

void Actor_register_default_defs() {
    pthread_mutexattr_init(&actor_mutex_attr);
    pthread_mutexattr_settype(&actor_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&actor_mutex, &actor_mutex_attr);

    Actor_register_def("test_actor", &debug_actor_init, &debug_actor_think, &debug_actor_render, &empty_render, &debug_actor_event, &debug_actor_recalc_bb);
    Actor_register_def("tilemap", &tilemap_actor_init, &tilemap_actor_think, &tilemap_actor_render, &empty_render, &tilemap_actor_event, &tilemap_actor_recalc_bb);
    Actor_register_def("viewport", &viewport_actor_init, &viewport_actor_think, &viewport_actor_render, &viewport_actor_late_render, &viewport_actor_event, &viewport_actor_recalc_bb);
    Actor_register_def("actor", &empty_init, &empty_think, &empty_render, &empty_render, &empty_event, &empty_recalc_bb);
    Actor_register_def("editor", &editor_actor_init, &editor_actor_think, &editor_actor_render, &editor_actor_late_render, &editor_actor_event, &editor_actor_recalc_bb);

    Actor_register_serialization_func("test_actor", &debug_actor_serialize);
    Actor_register_deserialization_func("test_actor", &debug_actor_deserialize);

    Actor_register_serialization_func("tilemap", &tilemap_actor_serialize);
    Actor_register_deserialization_func("tilemap", &tilemap_actor_deserialize);

    Actor_register_serialization_func("viewport", &viewport_actor_serialize);
    Actor_register_deserialization_func("viewport", &viewport_actor_deserialize);

    Actor_register_serialization_func("actor", &empty_serialize);
    Actor_register_deserialization_func("actor", &empty_deserialize);

    Actor_register_serialization_func("editor", &editor_actor_serialize);
    Actor_register_deserialization_func("editor", &editor_actor_deserialize);
}

transform_s Actor_get_transform(actor_s* actor) {
    float xPos = actor->transform.position.x;
    float yPos = actor->transform.position.y;

    float xScl = actor->transform.scale.x;
    float yScl = actor->transform.scale.y;

    float rot = actor->transform.rotation;

    actor_s *parent = actor->parent;

    while (parent && !actor->transform.top_level) {
        xPos += parent->transform.position.x;
        yPos += parent->transform.position.y;

        xScl *= parent->transform.scale.x;
        yScl *= parent->transform.scale.y;

        rot += actor->transform.rotation;

        if(parent->transform.top_level) break;
        parent = parent->parent;
    }

    return (transform_s){.rotation = rot, .scale = (float2_s){.x = xScl, .y = yScl}, .position = (float2_s){.x = xPos, .y = yPos}, .top_level = actor->transform.top_level};
}

transform_s Actor_get_transform_lerp(actor_s* actor, float time_in_tick) {
    transform_s local_lerped_tr = Actor_get_local_transform_lerp(actor, time_in_tick);

    float xPos = local_lerped_tr.position.x;
    float yPos = local_lerped_tr.position.y;

    float xScl = local_lerped_tr.scale.x;
    float yScl = local_lerped_tr.scale.y;

    float rot = local_lerped_tr.rotation;

    actor_s *parent = actor->parent;

    while (parent && !local_lerped_tr.top_level) {
        transform_s parent_lerped_tr = Actor_get_local_transform_lerp(parent, time_in_tick);
        xPos += parent_lerped_tr.position.x;
        yPos += parent_lerped_tr.position.y;

        xScl *= parent_lerped_tr.scale.x;
        yScl *= parent_lerped_tr.scale.y;

        rot += parent_lerped_tr.rotation;

        if(parent_lerped_tr.top_level) break;
        parent = parent->parent;
    }

    return (transform_s){.rotation = rot, .scale = (float2_s){.x = xScl, .y = yScl}, .position = (float2_s){.x = xPos, .y = yPos}, .top_level = actor->transform.top_level};
}

transform_s Actor_get_local_transform_lerp(actor_s* actor, float time_in_tick) {
    float PxPos = actor->pre_transform.position.x;
    float PyPos = actor->pre_transform.position.y;

    float PxScl = actor->pre_transform.scale.x;
    float PyScl = actor->pre_transform.scale.y;

    float Prot = actor->pre_transform.rotation;

    float xPos = actor->transform.position.x;
    float yPos = actor->transform.position.y;

    float xScl = actor->transform.scale.x;
    float yScl = actor->transform.scale.y;

    float rot = actor->transform.rotation;

    return (transform_s) {
        .rotation = Lerp(Prot, rot, time_in_tick),
        .scale = (float2_s){.x = Lerp(PxScl, xScl, time_in_tick), .y = Lerp(PyScl, yScl, time_in_tick)},
        .position = (float2_s){.x = Lerp(PxPos, xPos, time_in_tick), .y = Lerp(PyPos, yPos, time_in_tick)},
        .top_level = actor->transform.top_level,
    };
}

rect Actor_get_bounding_box(actor_s* actor) {
    if(!actor) return RECT_DEFAULT;
    if(!actor->bb) actor->recalc_bb(actor);
    if(!actor->bb) return RECT_DEFAULT;
    return *actor->bb;
}

string* Actor_serialize(void* serialized_obj) { // NOLINT(*-no-recursion)
    if(!serialized_obj) return s("nil");
    actor_s* actor = serialized_obj;
    string* ret_str = s("");

    s_cat(ret_str, so("transform {\n\t"));
    s_cat(ret_str, S_replace_n(Transform_serialize(&actor->transform), so("\n"), so("\n\t"), -1));
    s_cat(ret_str, so("} transform\n"));

    generic_serialize_value(actor, visible, bool, "visible");
    generic_serialize_value(actor, process, bool, "process");
    generic_serialize_value_ptr(actor, actor_id, char*, "id");

    generic_custom_serialize_obj_ptr(actor->data, actor->actor_id, data, Actor_get_serialization_func(actor->actor_id));

    generic_serialize_value_ptr(actor, name, char*, "name");

    if(actor->script) {
        generic_serialize_value_ptr(actor, script->script_path, char*, "script_path");
    }

    if(actor->children) {
        generic_serialize_value(actor, children->element_count, int, "child_count");

        for (int i = 0; i < actor->children->element_count; i++) {
            s_cat(ret_str, so("child {\n\t"));
            s_cat(ret_str, S_replace_n(Actor_serialize(actor->children->array[i]), so("\n"), so("\n\t"), -1));
            s_cat(ret_str, so("} actor\n"));
        }
    }


    return ret_str;
}

void* Actor_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    if(!actor_stack) {
        actor_stack = Stack_init(16384);
    }

    actor_s* actor = malloc(sizeof(actor_s));
    actor->children = list_new(sizeof(actor_s*));
    actor->ticks_since_spawn = 0;
    actor->visible = true;
    actor->process = true;
    actor->parent = nullptr;
    actor->script = nullptr;

    generic_deserialize_begin("actor")
        deserialize_stage_0()

        deserialize_stage_1({
            if (strcmp(name_buf, "transform") == 0) {
                if (strcmp(value_buf, "{") == 0) {
                    actor->transform = *(transform_s *) Deserialize_block_to_obj(
                            S_convert((str->c_str + marcher - 1)));
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
            if (strcmp(name_buf, "data") == 0) {
                if (strcmp(value_buf, "{") == 0) {
                    actor->data = Deserialize_block_to_specific_obj(S_convert((str->c_str + marcher - 1)), Actor_get_deserialization_func(actor->actor_id));
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
            if (strcmp(name_buf, "child") == 0) {
                if (strcmp(value_buf, "{") == 0) {
                    Actor_add_child(actor, Deserialize_block_to_obj(S_convert((str->c_str + marcher - 1))));
                    unsigned block_count = 1;
                    while (block_count) {
                        marcher++;
                        if (str->c_str[marcher] == '{')block_count++;
                        if (str->c_str[marcher] == '}')block_count--;
                    }
                    while (str->c_str[marcher] != '\n')marcher++;
                    stage = -1;
                    used = true;
                }
            }
        });

        deserialize_stage_2({
            if (strcmp(name_buf, "child") == 0) {
                Log_printf(LOG_LEVEL_INFO, "what");
            }
            if (strcmp(name_buf, "name") == 0) {
                actor->name = Deserialize(S_convert(value_buf), Get_deserialization_func("char*"));
                used = true;
            }
            if (strcmp(name_buf, "id") == 0) {
                actor->actor_id = Deserialize(S_convert(value_buf), Get_deserialization_func("char*"));
                used = true;
            }
            if (strcmp(name_buf, "visible") == 0) {
                bool *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                actor->visible = *ptr;
                free(ptr);
                used = true;
            }
            if (strcmp(name_buf, "process") == 0) {
                bool *ptr = Deserialize(S_convert(value_buf), Get_deserialization_func(parser_buf));
                actor->process = *ptr;
                free(ptr);
                used = true;
            }
            if (strcmp(name_buf, "script_path") == 0) {
                char *path = Deserialize(S_convert(value_buf), Get_deserialization_func("char*"));
                actor->script = Lua_script_load(path);
                used = true;
            }
        });
    generic_deserialize_end()

    actor_def_s* def = Actor_get_def(actor->actor_id);

    actor->event = def->event;
    actor->render = def->render;
    actor->late_render = def->late_render;
    actor->thinker = def->thinker;
    actor->init = def->init;
    actor->recalc_bb = def->recalc_bb;

    actor->handle = Stack_push(actor_stack, actor);

    return actor;
}

actor_s* Actor_from_path(const char* path) { // NOLINT(*-no-recursion)
    return Actor_from_path_rel(Engine_get_actors(), path);
//    actor_s* actors = Engine_get_actors();
//    char* this_path = strdup(path);
//    char* check_name = strdup(this_path);
//    unsigned long path_len = strlen(path);
//    int idx = 0;
//    while(check_name[idx++] != '/' && idx < path_len) continue;
//    check_name[idx + ((path_len == idx) ? 0 : -1)] = '\0';
//    if(strcmp(actors->name, check_name) != 0) {
//        return nullptr;
//    }
//    int start_idx = idx;
//    if(start_idx == path_len) {
//        return actors;
//    }
//    while(true) {
//        free(check_name);
//        if(!actors->children) return nullptr;
//        if(start_idx == path_len) {
//            return actors;
//        }
//
//        check_name = strdup(this_path);
//        idx = start_idx;
//        char* new_check = check_name + idx;
//        while(check_name[idx++] != '/' && idx < path_len) continue;
//        new_check[(idx - start_idx) + ((path_len == idx) ? 0 : -1)] = '\0';
//        start_idx = idx;
//        bool match_found = false;
//        for(int i = 0; i < actors->children->element_count; i++) {
//            actor_s* this = actors->children->array[i];
//            if(strcmp(this->name, new_check) == 0) {
//                actors = this;
//                match_found = true;
//            }
//        }
//        if(!match_found) break;
//    }
//    return nullptr;
}

actor_s* Actor_from_path_rel(actor_s* actor, const char* path) { // NOLINT(*-no-recursion)
    if(path[0] == '/') {
        return Actor_from_path(path + 1);
    }
    actor_s* actors = actor;
    char* this_chunk = strdup(path);
    unsigned long path_len = strlen(path);

    int idx = 0;
    while(this_chunk[idx++] != '/' && idx < path_len) continue;
    this_chunk[idx + ((path_len == idx) ? 0 : -1)] = '\0';

    if(strcmp(actor->name, this_chunk) == 0) {
        if(path_len == idx) {
            free(this_chunk);
            return actor;
        }
        free(this_chunk);
        return Actor_from_path_rel(actor, path + idx);
    }

    if(actors->children) {
        for (int i = 0; i < actors->children->element_count; i++) {
            actor_s *this = actors->children->array[i];
            if (strcmp(this->name, this_chunk) == 0) {
                if(path_len == idx) {
                    free(this_chunk);
                    return this;
                }
                free(this_chunk);
                return Actor_from_path_rel(this, path + idx);
            }
        }
    }

    if(strcmp(this_chunk, "..") == 0 && actor->parent) {
        free(this_chunk);
        return Actor_from_path_rel(actor->parent, path + idx);
    }

    char* actor_path = Actor_get_path(actor);
    Log_printf(LOG_LEVEL_ERROR, "Actor not found! %s relative to %s", path, actor_path);
    free(actor_path);
    free(this_chunk);
    return nullptr;
}

char* Actor_get_path(actor_s* actor) {
    string* path = s(actor->name);
    actor_s* parent = actor->parent;

    while(parent) {
        path = S_append(S_append(sc(parent->name), so("/")), path);
        parent = parent->parent;
    }

    // We use a string cause it makes our lives a little easier, but we're only returning a char*
    // To prevent the lil bit of memory that the string holds outside the char* from leaking
    // Once we're done, we just go ahead and copy the char* out from the string and free it.
    char* final_path = path->c_str;
    free(path);

    return final_path;
}

serialization_func(Actor_get_serialization_func(const char* id)) {
    if(!registered_actor_serialization_types) return nullptr;
    return map_get(registered_actor_serialization_types, id, serialization_func());
}

deserialization_func(Actor_get_deserialization_func(const char* id)) {
    if(!registered_actor_deserialization_types) return nullptr;
    return map_get(registered_actor_deserialization_types, id, deserialization_func());
}

void Actor_register_serialization_func(char* id, serialization_func(serializer)) {
    if(!registered_actor_serialization_types) {
        registered_actor_serialization_types = malloc(sizeof(serialization_func()));
        Map_init(registered_actor_serialization_types);
    }
    map_set(registered_actor_serialization_types, id, serializer);
}

void Actor_register_deserialization_func(char* id, deserialization_func(deserializer)) {
    if(!registered_actor_deserialization_types) {
        registered_actor_deserialization_types = malloc(sizeof(deserialization_func()));
        Map_init(registered_actor_deserialization_types);
    }
    map_set(registered_actor_deserialization_types, id, deserializer);
}