#ifndef CENGINE_SERIALIZATION_TESTS_H
#define CENGINE_SERIALIZATION_TESTS_H

#include "../serialization.h"
#include "../string.h"
#include "../errors.h"
#include "../log.h"

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
    generic_serialize_value_ptr(string_val, char*);

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
                generic_deserialize_obj_ptr_early(next_ptr);
        )

        deserialize_stage_2(
            generic_deserialize_obj_ptr(next_ptr);
            generic_deserialize_value(int_val, int);
            generic_deserialize_value(boolean_val, bool);
            generic_deserialize_value(float_val, float);
            str_deserialize_value(string_val);
        )
    generic_deserialize_end()

    return obj;
}

test_result* serialization_test_run(int* num_results) {
    start_test(serialization test)

    // Make sure serialization is set up and ready to go
    Register_base_serialization_types();
    Register_serialization_func("test_obj", &test_obj_serialize);
    Register_deserialization_func("test_obj", &test_obj_deserialize);

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

    file_s test_file = FS_open_dated("/tests/%s.serialization.test_result", nullptr, FILE_WRITE);

    file_ready_serialize(test_obj, &obj0, serialized_data);

    submit_test_result("Serialized data matches expected output", strcmp(serialized_data->c_str, "{\n"
                                                                                                 "\tnext_ptr {\n"
                                                                                                 "\t\tnext_ptr nil test_obj\n"
                                                                                                 "\t\tint_val 6 int\n"
                                                                                                 "\t\tboolean_val 1 bool\n"
                                                                                                 "\t\tfloat_val 42.000000 float\n"
                                                                                                 "\t\tstring_val Hello,\u200Bworld! char*\n"
                                                                                                 "\t} test_obj\n"
                                                                                                 "\tint_val 9 int\n"
                                                                                                 "\tboolean_val 0 bool\n"
                                                                                                 "\tfloat_val 69.000000 float\n"
                                                                                                 "\tstring_val Heyo,\u200Bworld! char*\n"
                                                                                                 "} test_obj") == 0);

    fprintf(test_file.f_ptr, "%s", serialized_data->c_str);

    FS_close(test_file);

    ref_dec(&serialized_data->refcount);

    end_test(num_results);
}

test_result* deserialization_test_run(int* num_results) {
    start_test(deserialization test)
    // Make sure serialization is set up and ready to go
    Register_base_serialization_types();
    Register_serialization_func("test_obj", &test_obj_serialize);
    Register_deserialization_func("test_obj", &test_obj_deserialize);

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

    string* working_dir = sc(FS_get_working_dir());
    s_cat(working_dir, so("/tests"));

    struct stat st = {0};

    if (stat(working_dir->c_str, &st) == -1) {
        mkdir(working_dir->c_str, 0700);
    }

    s_cat(working_dir, so("/input.serialization.test_result"));

    FILE* test_file = fopen(working_dir->c_str, "r");

    ref_dec(&working_dir->refcount);

    test_obj* obj2 = Deserialize_file_to_obj(test_file);

    submit_test_result("Top level integer matches", obj2->int_val == obj0.int_val);
    submit_test_result("Top level float matches", obj2->float_val == obj0.float_val);
    submit_test_result("Top level boolean matches", obj2->boolean_val == obj0.boolean_val);
    submit_test_result("Top level string matches", strcmp(obj2->string_val, obj0.string_val) == 0);
    submit_test_result("Top level has child node", obj2->next_ptr);
    if(obj2->next_ptr) {
        submit_test_result("Second level integer matches", obj2->next_ptr->int_val == obj1.int_val);
        submit_test_result("Second level float matches", obj2->next_ptr->float_val == obj1.float_val);
        submit_test_result("Second level boolean matches", obj2->next_ptr->boolean_val == obj1.boolean_val);
        submit_test_result("Second level string matches", strcmp(obj2->next_ptr->string_val, obj1.string_val) == 0);
        submit_test_result("Second level doesn't have child node", !obj2->next_ptr->next_ptr);
    }

    fclose(test_file);

    ref_dec(&obj0.refcount);
    ref_dec(&obj1.refcount);
    ref_dec(&obj2->refcount);
    end_test(num_results);
}
#endif //CENGINE_SERIALIZATION_TESTS_H
