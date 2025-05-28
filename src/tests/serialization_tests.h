#ifndef CENGINE_SERIALIZATION_TESTS_H
#define CENGINE_SERIALIZATION_TESTS_H

#include "../serialization.h"
#include "../string.h"

#include <string.h>
#include <malloc.h>
#include <sys/stat.h>

typedef struct test_obj {
    struct test_obj* next_ptr;
    int int_val;
    bool boolean_val;
    float float_val;
    char* string_val;
    ref refcount;
} test_obj;

static void test_obj_free(const ref *ref) {
    ref_counted_free_begin(test_obj, obj);
    if(obj->next_ptr) ref_dec(&obj->next_ptr->refcount);
    free(obj->string_val);
    ref_counted_free_end(obj);
}

// test_obj serializer defs
string* test_obj_serialize(void* serialized_obj) {
    if(!serialized_obj) return s("nil");
    test_obj* obj = serialized_obj;
    string* ret_str = s("");

    generic_serialize_obj_ptr(next_ptr, test_obj, 5);
    generic_serialize_value(int_val, int);
    generic_serialize_value(boolean_val, bool);
    generic_serialize_value(float_val, float);
    generic_serialize_value(string_val, char*);

    return ret_str;
}

void* test_obj_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    test_obj* obj = malloc(sizeof(test_obj));
    obj->refcount = (struct ref){test_obj_free, 1};

    generic_deserialize_begin()
        deserialize_stage_0()

        deserialize_stage_1(
            if(strcmp(name_buf, "next") == 0) {
                if(strcmp(value_buf, "{") == 0) {
                    obj->next_ptr = J_block_to_obj(sc((str->c_str + marcher - 1)));
                    unsigned block_count = 1;
                    while(block_count) {
                        marcher++;
                        if(str->c_str[marcher] == BLOCK_BEGIN) block_count++;
                        if(str->c_str[marcher] == BLOCK_END) block_count--;
                    }
                    while(str->c_str[marcher] != '\n') marcher++;
                    stage = -1;
                }
            }
        )

        deserialize_stage_2(
            if(strcmp(name_buf, "next") == 0) {
                if(strcmp(value_buf, "nil") == 0) {
                    obj->next_ptr = nullptr;
                } else {
                    obj->next_ptr = deserialize(test_obj, sc(value_buf));
                }
            }

            generic_deserialize_value(int_val, int);
            generic_deserialize_value(boolean_val, bool);
            generic_deserialize_value(float_val, float);
            str_deserialize_value(string_val);
        )
    generic_deserialize_end()

    return obj;
}

void serialization_test_run() {

    // Make sure serialization is set up and ready to go
    J_register_base_types();
    J_register_serialization_func("test_obj", &test_obj_serialize);
    J_register_deserialization_func("test_obj", &test_obj_deserialize);

    test_obj obj0;
    obj0.int_val = 9;
    obj0.boolean_val = false;
    obj0.float_val = 69.0f;
    obj0.string_val = "Heyo, world!";

    test_obj obj1;
    obj1.int_val = 6;
    obj1.boolean_val = true;
    obj1.float_val = 42.0f;
    obj1.string_val = "Hello, world!";
    obj1.next_ptr = nullptr;

    obj0.next_ptr = &obj1;

    string* working_dir = sc(F_get_working_dir());
    s_cat(working_dir, so("/tests"));

    struct stat st = {0};

    if (stat(working_dir->c_str, &st) == -1) {
        mkdir(working_dir->c_str, 0700);
    }

    const int date_str_len = 20;

    time_t current_time = time(nullptr);
    char* date_string = malloc(sizeof(char) * date_str_len);
    strftime(date_string, date_str_len, "%Y-%j-%H-%M-%S", localtime(&current_time));

    s_cat(working_dir, so("/"));
    s_cat(working_dir, sco(date_string));
    s_cat(working_dir, so(".serialization.test_result"));

    FILE* test_file = fopen(working_dir->c_str, "w");

    ref_dec(&working_dir->refcount);

    file_ready_serialize(test_obj, &obj0, serialized_data);

    fprintf(test_file, "%s", serialized_data->c_str);
    fflush(test_file);
    fclose(test_file);

    ref_dec(&serialized_data->refcount);
}

void deserialization_test_run() {
    // Make sure serialization is set up and ready to go
    J_register_base_types();
    J_register_serialization_func("test_obj", &test_obj_serialize);
    J_register_deserialization_func("test_obj", &test_obj_deserialize);

    string* working_dir = sc(F_get_working_dir());
    s_cat(working_dir, so("/tests"));

    struct stat st = {0};

    if (stat(working_dir->c_str, &st) == -1) {
        mkdir(working_dir->c_str, 0700);
    }

    s_cat(working_dir, so("/input.serialization.test_result"));

    FILE* test_file = fopen(working_dir->c_str, "r");

    ref_dec(&working_dir->refcount);

    test_obj* obj0 = J_file_to_obj(test_file);

    fclose(test_file);

    ref_dec(&obj0->refcount);
}
#endif //CENGINE_SERIALIZATION_TESTS_H
