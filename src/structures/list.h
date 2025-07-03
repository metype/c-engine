#ifndef CENGINE_LIST_H
#define CENGINE_LIST_H

#include <stddef.h>

#define list_add(list, elm, ret) do { \
    if(!list) {                  \
        list = malloc(sizeof(list_s));                            \
    }\
    if(!list->array) {                                                                  \
        list->array_size = 1;                                                           \
        list->array = malloc(list->element_type_size);                                   \
    }                                                                                  \
    if(list->element_count >= list->array_size) {                                        \
        list->array_size *= 2;                                                          \
        void* new_arr = realloc(list->array, list->element_type_size * list->element_type_size); \
        assert(new_arr, "realloc failed, out of memory.", ret);                    \
        list->array = new_arr;                                                          \
    }                                                                                  \
    list->array[list->element_count++] = elm;                                            \
} while(0)

#define list_remove(list, elm) do { \
    void** new_arr = malloc(list->element_type_size * list->array_size);       \
    register int idx = 0;                                                    \
    for(register int i = 0; i < list->element_count; i++) {                   \
        if(list->array[i]) {                                                  \
            new_arr[idx++] = list->array[i];                                  \
        }                                                                    \
    }                                                                        \
    void** reallocated_arr = realloc(new_arr, list->element_type_size * idx); \
    free(new_arr);                                                           \
    if(!reallocated_arr) {                                                   \
        Log_print(LOG_LEVEL_ERROR, "Out of memory.");                        \
        return;                                                              \
    }                                                                        \
    free(list->array);                                                        \
    list->array_size = idx;                                                   \
    list->array = reallocated_arr;                                            \
} while(0)

typedef struct list {
    size_t element_type_size;
    size_t element_count;
    size_t array_size;
    void** array;
} list_s;

list_s* list_new(size_t type_size);

#endif //CENGINE_LIST_H
