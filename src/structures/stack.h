#ifndef CENGINE_STACK_H
#define CENGINE_STACK_H

#include "../errors.h"
#include "../log.h"

typedef struct stack_s {
    void** data;
    int length;
    int idx;
} stack;

stack* Stack_init(int len);
inline int Stack_push(stack* stack, void* val) {
    assert(stack->idx < stack->length, "Stack overflow.", return 0);
    stack->data[stack->idx++] = val;
    return stack->idx - 1;
}

inline void* Stack_pull(stack* stack) {
    if(stack->idx <= 0) {
        return nullptr;
    }
    return stack->data[--stack->idx];
}

inline void* Stack_get(stack* stack, int idx)  {
    assert(idx >= 0 && idx < stack->length, "Stack oob.", return nullptr);
    return stack->data[idx];
}

inline void Stack_set(stack* stack, int idx, void* val) {
    assert(idx >= 0 && idx < stack->length, "Stack oob.", return);
    stack->data[idx] = val;
}
#endif //CENGINE_STACK_H
