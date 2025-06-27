#ifndef CENGINE_REF_H
#define CENGINE_REF_H
#pragma once

#include <stdatomic.h>
#include <stddef.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define ref_counted_free_begin(type, name) type* name = container_of(ref, type, refcount)

#define ref_counted_free_end(name) do { free(name); } while(0)

typedef struct ref {
    void (*free)(const struct ref *);
    _Atomic int count;
} ref;

static inline void
ref_inc(const ref *ref)
{
    atomic_fetch_add((_Atomic int*)&ref->count, 1);
}

static inline void
ref_dec(const ref *ref)
{
    if (atomic_fetch_sub((_Atomic int*)&ref->count, 1) == 1)
        ref->free(ref);
}


#endif //CENGINE_REF_H
