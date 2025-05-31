#ifndef CENGINE_TEST_SUITE_H
#define CENGINE_TEST_SUITE_H
#include "../callbacks.h"

typedef struct test_result {
    bool success;
    char* test_name;
} test_result;

typedef struct test {
    CALLBACK(test_func, test_result*, int*);
    char* test_name;
} test;

#define start_test(test_name) \
    test_result* results = malloc(sizeof(test_result)); \
    int test_result_arr_length = 1;                     \
    int test_result_arr_idx = 0;                        \

#define submit_test_result(name, success_val) do {                                       \
    if(test_result_arr_idx >= test_result_arr_length) {                                  \
        test_result_arr_length++;                                                        \
        void* new_result_list = realloc(results, sizeof(test) * test_result_arr_length); \
        assert(new_result_list != nullptr, "Result list cannot grow!");                  \
        results = new_result_list;                                                       \
    }                                                                                    \
    results[test_result_arr_idx].success = success_val;                                  \
    results[test_result_arr_idx].test_name = name;                                       \
    test_result_arr_idx++;                                                               \
} while(0)

#define end_test(result_count_ptr) do {                                      \
    if(result_count_ptr != nullptr) *result_count_ptr = test_result_arr_idx; \
    return results;                                                          \
} while(0)

void T_register_test(test test_to_register);

void T_run_all_tests();

#endif //CENGINE_TEST_SUITE_H
