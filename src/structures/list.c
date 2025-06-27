#include "list.h"
#include <malloc.h>

list_s* list_new(size_t type_size) {
    list_s* list = malloc(sizeof(list_s));
    *list = (list_s){ .element_type_size = type_size, .element_count = 0, .array_size = 0, .array = nullptr };
    return list;
}