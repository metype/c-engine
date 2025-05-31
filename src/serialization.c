#include "definitions.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "serialization.h"
#include "log.h"
#include "hashmap.h"
#include "string.h"

hash_map_s* registered_serialization_types = nullptr;
hash_map_s* registered_deserialization_types = nullptr;

void Register_serialization_func(char* id, serialization_func(serializer)) {
    if(!registered_serialization_types) {
        registered_serialization_types = malloc(sizeof(serialization_func()));
        Map_init(registered_serialization_types);
    }
    map_set(registered_serialization_types, id, serializer);
}

void Register_deserialization_func(char* id, deserialization_func(deserializer)) {
    if(!registered_deserialization_types) {
        registered_deserialization_types = malloc(sizeof(deserialization_func()));
        Map_init(registered_deserialization_types);
    }
    map_set(registered_deserialization_types, id, deserializer);
}

serialization_func(Get_serialization_func(const char* id)) {
    if(!registered_serialization_types) return nullptr;
    return map_get(registered_serialization_types, id, serialization_func());
}

deserialization_func(Get_deserialization_func(const char* id)) {
    if(!registered_deserialization_types) return nullptr;
    return map_get(registered_deserialization_types, id, deserialization_func());
}

void* Deserialize_file_to_obj(FILE* file) {
    char* buf = nullptr;
    long bufsize;
    if (file != nullptr) {
        /* Go to the end of the file. */
        if (fseek(file, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            bufsize = ftell(file);
            if (bufsize == -1) { return nullptr; }

            /* Allocate our buffer to that size. */
            buf = malloc(sizeof(char) * (bufsize + 1));

            /* Go back to the start of the file. */
            if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fread(buf, sizeof(char), bufsize, file);
            if ( ferror( file ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                buf[newLen++] = '\0'; /* Just to be safe. */
            }
        }
    } else return nullptr;
    if(buf == nullptr) return nullptr;
    string* file_data = sc(buf);
    s_rep(file_data, so("\t"), so(""));

    return Deserialize_block_to_obj(file_data);
}

void* Deserialize_block_to_obj(string* str) {
    int block_begin = -1;
    unsigned block_end = 0;
    unsigned block_count = 0;

    for(int i = 0; i < S_length(str); i++ ) {
        if (str->c_str[i] == BLOCK_BEGIN) {
            block_count++;
            if (block_begin < 0) block_begin = i + 2;
        }
        if (str->c_str[i] == BLOCK_END) {
            block_count--;
            if (block_count == 0) {
                block_end = i;
                break;
            }
        }
    }

    if(block_count != 0) {
        Log_printf(LOG_LEVEL_ERROR, "Failed to parse block! Block not closed! Raw Block Data: \"%s\"", str->c_str);
        return nullptr;
    }

    char type_buf[64];
    unsigned idx = block_end + 2;
    unsigned buf_idx = 0;
    while(true) {
        if(buf_idx >= 64) {
            Log_print(LOG_LEVEL_ERROR, "Failed to parse! Type specifier exceeded length limit!");
            return nullptr;
        }
        if(str->c_str[idx] == ' ' || str->c_str[idx] == '\n' || str->c_str[idx] == 0) break;
        type_buf[buf_idx++] = str->c_str[idx++];
    }
    type_buf[buf_idx] = 0;

    deserialization_func(deserializer) = Get_deserialization_func(type_buf);

    if(!deserializer) {
        Log_print(LOG_LEVEL_ERROR, "Failed to parse! Type specifier does not have properly registered funcs!");
        return nullptr;
    }

    char* block_buf = malloc(sizeof(char) * (block_end - block_begin + 2));

    idx = 0;
    for(int i = block_begin; i < block_end; i++) {
        block_buf[idx++] = str->c_str[i];
    }
    block_buf[idx] = 0;

    string* block_str_data = sc(block_buf);
    return Deserialize(block_str_data, deserializer);
}

string* Serialize(void* obj, serialization_func(serializer)) {
    if(!serializer) return nullptr;
    if(!obj) return nullptr;
    return serializer(obj);
}

void* Deserialize(string* obj_str, deserialization_func(deserializer)) {
    if(!deserializer) return nullptr;
    if(!obj_str) return nullptr;
    return deserializer(obj_str);
}

#define generic_serialize(type, generic, buf_size, conversion_fmt_str) string* CONCAT(type, _serialize)(void* obj) { \
    if(!obj) return generic;                                                   \
    type value = *(type*)obj;                                                  \
    char* value_buf = malloc(sizeof(char) * (buf_size));                         \
    snprintf(value_buf, buf_size, conversion_fmt_str, value);                  \
    return sc(value_buf);                                                      \
}

#define generic_deserialize(type, conversion_expr) void* CONCAT(type, _deserialize)(string* str) { \
    if(!str) return nullptr;                              \
    if(!str->c_str) return nullptr;                       \
    type* val_ptr = malloc(sizeof(type));                 \
    *val_ptr = (type) (conversion_expr);                  \
    return val_ptr;                                       \
}


// Int serializer defs
generic_serialize(int, s("0"), 24, "%i")
generic_deserialize(int, strtol(str->c_str, nullptr, 10))

// Bool serializer defs
generic_serialize(bool, s("0"), 2, "%i")
generic_deserialize(bool, strtol(str->c_str, nullptr, 10))

// Float serializer defs
generic_serialize(float, s("0.0"), 64, "%f")
generic_deserialize(float, strtof(str->c_str, nullptr))

// char* serializer defs
string* str_serialize(void* obj) {
    if(!obj) return s("");
    char* value = (char*)obj;
    unsigned len = strlen(value) + 1;
    char* value_buf = malloc(sizeof(char) * len);
    snprintf(value_buf, len, "%s", value);
    string* str = sc(value_buf);
    s_rep(str, so(" "), so("\u200B"));
    return str;
}

void* str_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;
    string* new_str = S_replace(str, so("\u200B"), so(" "));
    unsigned len = (S_length(new_str) + 1);
    char* val_ptr = malloc(sizeof(char) * len);
    snprintf(val_ptr, len, "%s", new_str->c_str);
    return val_ptr;
}

void Register_base_serialization_types() {
    Register_serialization_func("int", &int_serialize);
    Register_deserialization_func("int", &int_deserialize);

    Register_serialization_func("bool", &bool_serialize);
    Register_deserialization_func("bool", &bool_deserialize);

    Register_serialization_func("float", &float_serialize);
    Register_deserialization_func("float", &float_deserialize);

    Register_serialization_func("char*", &str_serialize);
    Register_deserialization_func("char*", &str_deserialize);
}