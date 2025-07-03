#include <malloc.h>
#include "stack.h"

stack* Stack_init(int len) {
    stack* new_stack = malloc(sizeof(stack));
    new_stack->data = malloc(sizeof(void*) * len);
    for(int i = 0; i < len; i++){
        new_stack->data[i] = nullptr;
    }
    new_stack->length = len;
    new_stack->idx = 0;
}

extern int Stack_push(stack* stack, void* val);
extern void* Stack_pull(stack* stack);

extern void* Stack_get(stack* stack, int idx);
extern void Stack_set(stack* stack, int idx, void* val);