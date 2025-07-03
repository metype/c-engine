#ifndef CENGINE_SERIALIZATION_H
#define CENGINE_SERIALIZATION_H

#include <stdio.h>

#define BLOCK_BEGIN '{'
#define BLOCK_END '}'

#if defined(__linux__)
    #include <bits/types/FILE.h>
#elif defined(__WIN32) || defined(_WIN32_WINNT)
    #include "win32_file_wrapper.h"
#endif

#include "callbacks.h"
#include "definitions.h"

#define serialization_func(name) M_CALLBACK(name, string*, void*)
#define deserialization_func(name) M_CALLBACK(name, void*, string*)

#define serialize(type, obj) Serialize(obj, Get_serialization_func(#type))
#define deserialize(type, data) Deserialize(data, Get_deserialization_func(#type))

#define file_ready_serialize(type, obj, var) string* var = s("{\n"); \
s_cat(var, serialize(type, obj));                                    \
s_rep_n(var, so("\n"), so("\n\t"), -1);                              \
s_cat(var, so("} " #type));                                          \

#define generic_deserialize_obj_early(obj, val_key, val_name, val_type) do {            \
if(strcmp(name_buf, #val_name) == 0) {                                   \
    if(strcmp(value_buf, "{") == 0) {                                   \
        obj->val_key = *(val_type*)Deserialize_block_to_obj(sc((str->c_str + marcher - 1))); \
        unsigned block_count = 1;                                       \
        while(block_count) {                                            \
        marcher++;                                                      \
        if(str->c_str[marcher] == BLOCK_BEGIN) block_count++;           \
        if(str->c_str[marcher] == BLOCK_END) block_count--;             \
        used = true;                                                    \
    }                                                                   \
    while(str->c_str[marcher] != '\n') marcher++;                       \
    stage = -1;                                                         \
    }                                                                   \
}} while(0)

#define generic_deserialize_obj_ptr_early(obj, val_key, val_name, val_type) do {            \
if(strcmp(name_buf, #val_name) == 0) {                                   \
    if(strcmp(value_buf, "{") == 0) {                                   \
        obj->val_key = (val_type) Deserialize_block_to_obj(sc((str->c_str + marcher - 1))); \
        unsigned block_count = 1;                                       \
        while(block_count) {                                            \
        marcher++;                                                      \
        if(str->c_str[marcher] == BLOCK_BEGIN) block_count++;           \
        if(str->c_str[marcher] == BLOCK_END) block_count--;             \
        used = true;                                                    \
    }                                                                   \
    while(str->c_str[marcher] != '\n') marcher++;                       \
    stage = -1;                                                         \
    }                                                                   \
}} while(0)

#define generic_deserialize_obj(obj, val_type, val_key, val_name) do {                                                      \
if(strcmp(name_buf, #val_name) == 0) {                                                                \
    if(strcmp(value_buf, "nil") == 0) {                                                                \
        obj->val_key = (val_type){};                                                                       \
    } else {                                                                                           \
        obj->val_key = *(val_type*)Deserialize(sc(value_buf), Get_deserialization_func(parser_buf));              \
    }                                                                                                  \
}} while(0)

#define generic_deserialize_obj_ptr(obj, val_key, val_name) do {                                                      \
if(strcmp(name_buf, #val_name) == 0) {                                                                \
    if(strcmp(value_buf, "nil") == 0) {                                                                \
        obj->val_key = nullptr;                                                                       \
    } else {                                                                                           \
        obj->val_key = Deserialize(sc(value_buf), Get_deserialization_func(parser_buf));              \
    }                                                                                                  \
}} while(0)

#define generic_deserialize_value(obj, val_key, val_type, val_name) do { \
if(strcmp(name_buf, #val_name) == 0) {                     \
    val_type* ptr = Deserialize(sc(value_buf), Get_deserialization_func(parser_buf)); \
    obj->val_key = *ptr;                                  \
    free(ptr);                                                           \
    used = true;                                                                         \
}} while(0)

#define str_deserialize_value(obj, val_key, val_name) do {      \
if(strcmp(name_buf, #val_name) == 0) {                 \
    obj->val_key = deserialize(char*, sc(value_buf)); \
}} while(0)

#define generic_int_ser_val(accessor, val_name, val_type) do { \
    s_cat(ret_str, s(val_name " "));                           \
    s_cat(ret_str, serialize(val_type, accessor));             \
    s_cat(ret_str, s(" " #val_type "\n"));                     \
} while(0)

#define generic_serialize_value(obj, val_key, val_type, val_name) generic_int_ser_val(&obj->val_key, val_name, val_type)

#define generic_serialize_value_ptr(obj, val_key, val_type, val_name) generic_int_ser_val(obj->val_key, val_name, val_type)

#define generic_custom_serialize_obj_ptr(obj, val_type, val_name, serializer) do{                \
    string* data_str = Serialize(obj, serializer); \
    bool data_exist = true;\
    if(!data_str) data_exist = false;\
    if(data_exist && strcmp(data_str->c_str, "nil") == 0) data_exist = false;\
    \
    if(!data_exist) {\
        s_cat(ret_str, so(#val_name " nil "));\
    } else {\
        s_cat(ret_str, so(#val_name  " {\n\t"));\
        ret_str = S_append(S_final(ret_str), S_replace_n(data_str, so("\n"), so("\n\t"), -1));\
        s_cat(ret_str, so("} "));\
    }\
    s_cat(ret_str, S_final(S_append(sc(val_type), so("\n"))));\
} while(0)

#define generic_serialize_obj_ptr(obj, val_type, val_name) generic_custom_serialize_obj_ptr(obj, val_type, val_name, Get_serialization_func(val_type));

#define generic_deserialize_begin(name)                                     \
    const char* deserializer_name = name;\
    unsigned len = S_length(str);\
    unsigned marcher = 0;\
    unsigned buf_idx = 0;\
    char buffer[1024];\
    char name_buf[128];\
    char value_buf[128];\
    char parser_buf[128];\
    unsigned stage = 0;\
    bool used = false;                                                      \
    bool hit_non_newline = false;                                                                        \
    while (marcher < len) {                                                 \
        if(str->c_str[marcher] == '\n' && !hit_non_newline) {               \
            marcher++;                                                      \
            continue; \
        }                                                                    \
        if (str->c_str[marcher] != ' ' && str->c_str[marcher] != '\n') {    \
            hit_non_newline = true;                                                 \
            if (str->c_str[marcher] == '\t') {\
                marcher++;\
                continue;\
            }\
            if (buf_idx >= 1024) {\
                Log_print(LOG_LEVEL_ERROR, "Inevitable buffer overrun detected! You sneaky bugger! Get truncated!");\
                while(str->c_str[marcher] != ' ' && str->c_str[marcher] != '\n') marcher++;\
            } else {\
                buffer[buf_idx] = str->c_str[marcher];\
                buf_idx++;\
                marcher++;\
                continue;\
            }\
        }\
        buffer[buf_idx] = 0;\
        buf_idx = 0;\

#define generic_deserialize_end() \
                                 \
    stage++;                      \
    marcher++;                    \
}

#define deserialize_stage_0(expr)          \
if(stage == 0) {                           \
    snprintf(name_buf, 128, "%s", buffer); \
    expr                                   \
}

#define deserialize_stage_1(expr)           \
if(stage == 1) {                            \
    snprintf(value_buf, 128, "%s", buffer); \
    expr                                    \
}

#define deserialize_stage_2(expr)            \
if(stage == 2) {                             \
    snprintf(parser_buf, 128, "%s", buffer); \
    stage = -1;                              \
    expr                                     \
    if(!used) {                   \
        Log_printf(LOG_LEVEL_WARNING, "Key %s not used in object %s, but referenced in file?", name_buf, deserializer_name); \
        used = false;                                         \
    }                                        \
}


typedef struct string string;

void* Deserialize_file_to_obj(FILE* file);

void* Deserialize_block_to_obj(string* str);

void* Deserialize_block_to_specific_obj(string* str, deserialization_func(provided_deserializer));

serialization_func(Get_serialization_func(const char*));
deserialization_func(Get_deserialization_func(const char*));

string* Serialize(void* obj, serialization_func(serializer));
void* Deserialize(string* obj_str, deserialization_func(deserializer));

void Register_serialization_func(char* id, serialization_func(serializer));
void Register_deserialization_func(char* id, deserialization_func(deserializer));

void Register_base_serialization_types();

#endif //CENGINE_SERIALIZATION_H
