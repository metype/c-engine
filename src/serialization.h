#ifndef CENGINE_SERIALIZATION_H
#define CENGINE_SERIALIZATION_H

#define BLOCK_BEGIN '{'
#define BLOCK_END '}'

#include "callbacks.h"
#include "definitions.h"

#define serialization_func(name) CALLBACK(name, string*, void*)
#define deserialization_func(name) CALLBACK(name, void*, string*)

#define serialize(type, obj) J_serialize(obj, J_get_serialization_func_from_str(#type))
#define deserialize(type, data) J_deserialize(data, J_get_deserialization_func_from_str(#type))

#define file_ready_serialize(type, obj, var) string* var = s("{\n"); \
s_cat(var, serialize(type, obj));                           \
s_rep_n(var, so("\n"), so("\n\t"), -1);                              \
s_cat(var, so("} "));                                                \
s_cat(var, so(#type));                                               \

#define generic_deserialize_value(val_key, val_type) do { \
if(strcmp(name_buf, #val_key) == 0) { \
    val_type* ptr = deserialize(val_type, sc(value_buf)); \
    obj->val_key = *ptr; \
    free(ptr); \
}} while(0)

#define str_deserialize_value(val_key) do {\
if(strcmp(name_buf, #val_key) == 0) { \
    obj->val_key = deserialize(char*, sc(value_buf)); \
}} while(0)

#define generic_serialize_value(val_key, val_type) do {\
    s_cat(ret_str, s(#val_key " "));                           \
    s_cat(ret_str, serialize(val_type, &obj->val_key));    \
    s_cat(ret_str, s(" " #val_type "\n"));                 \
} while(0)

#define generic_serialize_obj_ptr(val_key, val_type, num_fields) do{ \
    s_cat(ret_str, (obj->val_key) ? so(#val_key " {\n") : so(#val_key " ")); \
    s_cat(ret_str, (obj->val_key) ? serialize(test_obj, obj->val_key) : so("nil")); \
    s_rep_n(ret_str, so("\n"), so("\n\t"), num_fields); \
    s_cat(ret_str, (obj->next_ptr) ? so("} test_obj\n") : so(" test_obj\n")); \
} while(0)

#define generic_deserialize_begin() \
    unsigned len = S_length(str); \
    unsigned marcher = 0; \
    unsigned buf_idx = 0; \
    char buffer[1024]; \
    \
    char name_buf[128]; \
    char value_buf[128]; \
    char parser_buf[128]; \
    \
    unsigned stage = 0; \
    \
    while(marcher < len) { \
        if(str->c_str[marcher] != ' ' && str->c_str[marcher] != '\n') { \
        if(str->c_str[marcher] == '\t') { \
        marcher++; \
        continue; \
        }\
        buffer[buf_idx] = str->c_str[marcher];\
        buf_idx++;\
        marcher++;\
        continue;\
        }\
        buffer[buf_idx] = 0;\
        buf_idx = 0;                \

#define generic_deserialize_end()\
    stage++; \
    marcher++; \
}

#define deserialize_stage_0(expr) \
if(stage == 0) {\
    snprintf(name_buf, 128, "%s", buffer); \
    expr                              \
}

#define deserialize_stage_1(expr) \
if(stage == 1) {\
    snprintf(value_buf, 128, "%s", buffer); \
    expr                              \
}

#define deserialize_stage_2(expr) \
if(stage == 2) {\
    snprintf(parser_buf, 128, "%s", buffer); \
    stage = -1; \
    expr                              \
}


typedef struct string string;

void* J_file_to_obj(FILE*);

void* J_block_to_obj(string* str);

serialization_func(J_get_serialization_func_from_str(const char*));
deserialization_func(J_get_deserialization_func_from_str(const char*));

string* J_serialize(void* obj, serialization_func(serializer));
void* J_deserialize(string* obj_str, deserialization_func(deserializer));

void J_register_serialization_func(char* id, serialization_func(serializer));
void J_register_deserialization_func(char* id, deserialization_func(deserializer));

void J_register_base_types();

#endif //CENGINE_SERIALIZATION_H
