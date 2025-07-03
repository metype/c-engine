#ifndef CENGINE_SERIALIZATION_TESTS_H
#define CENGINE_SERIALIZATION_TESTS_H

#include "../serialization.h"
#include "../string.h"
#include "../errors.h"
#include "../log.h"
#include "../definitions.h"
#include "../transform.h"

#if CENGINE_WIN32
#include "../win32_stdlib.h"
#endif

#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <float.h>
#include <math.h>

test_result* serialization_test_run(int* num_results) {
    start_test(serialization test)

    // Make sure serialization is set up and ready to go
    Register_base_serialization_types();
    Register_serialization_func("transform", &Transform_serialize);
    Register_deserialization_func("transform", &Transform_deserialize);

    transform_s tr = {
            .position = {
                    .x = 20.5f,
                    .y = 69.42f,
            },
            .rotation = 45.2f,
            .scale = {
                    .x = 1.7f,
                    .y = 7.3f,
            },
    };

    file_s test_file = FS_open_dated("/tests/%s.serialization.test_result", nullptr, FA_FILE_WRITE);

    file_ready_serialize(transform, &tr, serialized_data);

    submit_test_result("Serialized data matches expected output", strcmp(serialized_data->c_str, "{\n"
                                                                                                 "\tx 20.500000 float\n"
                                                                                                 "\ty 69.419998 float\n"
                                                                                                 "\tscale_x 1.700000 float\n"
                                                                                                 "\tscale_y 7.300000 float\n"
                                                                                                 "\trot 45.200001 float\n"
                                                                                                 "} transform") == 0);

    fprintf(test_file.f_ptr, "%s", serialized_data->c_str);

    FS_close(test_file);

    ref_dec(&serialized_data->refcount);

    end_test(num_results);
}

test_result* deserialization_test_run(int* num_results) {
    start_test(deserialization test)
    // Make sure serialization is set up and ready to go
    Register_base_serialization_types();
    Register_serialization_func("transform", &Transform_serialize);
    Register_deserialization_func("transform", &Transform_deserialize);

    transform_s tr = {
            .position = {
                    .x = 20.5f,
                    .y = 69.42f,
            },
            .rotation = 45.2f,
            .scale = {
                    .x = 1.7f,
                    .y = 7.3f,
            },
    };

    string* working_dir = sc(FS_get_working_dir());
    s_cat(working_dir, so("/tests"));

    FS_create_dir_if_not_exist(working_dir->c_str);

    s_cat(working_dir, so("/input.serialization.test_result"));

    FILE* test_file = fopen(working_dir->c_str, "r");

    ref_dec(&working_dir->refcount);

    transform_s * tr2 = Deserialize_file_to_obj(test_file);

    submit_test_result("Position X matches", fabsf(tr.position.x - tr2->position.x) < FLT_EPSILON);
    submit_test_result("Position Y matches", fabsf(tr.position.y - tr2->position.y) < FLT_EPSILON);
    submit_test_result("Scale X matches", fabsf(tr.scale.x - tr2->scale.x) < FLT_EPSILON);
    submit_test_result("Scale Y matches", fabsf(tr.scale.y - tr2->scale.y) < FLT_EPSILON);
    submit_test_result("Rotation matches", fabsf(tr.rotation - tr2->rotation) < FLT_EPSILON);

    fclose(test_file);
    end_test(num_results);
}
#endif //CENGINE_SERIALIZATION_TESTS_H
