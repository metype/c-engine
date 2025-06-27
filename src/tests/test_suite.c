#include <malloc.h>
#include "test_suite.h"
#include "../errors.h"
#include "../log.h"
#include "../win32_stdlib.h"

test* all_tests = nullptr;
unsigned test_list_idx = 0;
unsigned test_list_length = 0;

void check_grow_test_list() {
    if(test_list_idx >= test_list_length) {
        test_list_length *= 2;
        void* new_test_list = realloc(all_tests, sizeof(test) * test_list_length);
        assert(new_test_list != nullptr, "Test list cannot grow!", return);
        all_tests = new_test_list;
    }
}

void T_register_test(test test_to_register) {

    // Oh god, how I miss you std::vector<T>...
    // Oh god... how I simply miss <T>

    if(all_tests == nullptr) {
        all_tests = malloc(sizeof(test) * 8);
        test_list_length = 8;
    }
    check_grow_test_list();
    all_tests[test_list_idx++] = test_to_register;
}

void T_run_all_tests() {
    if(all_tests == nullptr) return;
    Log_printf(LOG_LEVEL_DEBUG, "Beginning test suite, there are %i tests to run!", test_list_idx);
    unsigned total_test_result_num = 0;
    unsigned successful_test_result_num = 0;
    for(unsigned i = 0; i < test_list_idx; i++) {
        Log_printf(LOG_LEVEL_DEBUG, "Running test %i/%i \"%s\"", i + 1, test_list_idx, all_tests[i].test_name);
        int num_results = 0;
        test_result* results = all_tests[i].test_func(&num_results);
        total_test_result_num += num_results;
        for(unsigned j = 0; j < num_results; j++) {
            successful_test_result_num += results[j].success;
            Log_printf(LOG_LEVEL_DEBUG, "Test result %u/%i \"%s\" %s", j + 1, num_results, results[j].test_name,
                       results[j].success ? "SUCCESS!" : "FAILED!");
        }
        free(results);
    }
    Log_printf(LOG_LEVEL_DEBUG,
               "Test suite complete! Ran %i tests which returned a total of %i results! There were %i/%i successful results, which is %3.2f%%",
               test_list_idx, total_test_result_num, successful_test_result_num, total_test_result_num,
               (((float) successful_test_result_num) / ((float) total_test_result_num)) * 100);
    if(test_list_idx > total_test_result_num) {
        Log_printf(LOG_LEVEL_WARNING,
                   "Do note that there were less results than tests, this indicates that something may have gone wrong! (%i < %i)",
                   total_test_result_num, test_list_idx);
    }
}