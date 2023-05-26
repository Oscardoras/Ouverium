#include <stdlib.h>

#include "data.h"


size_t max(size_t a, size_t b);
inline size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

__Array __Array_new(__VirtualTable* vtable, size_t capacity) {
    __Array array = {
        .tab = calloc(capacity, vtable->info.size),
        .size = 0,
        .capacity = capacity
    };
}

void* __Array_get(__ArrayInfo array, size_t i) {
    return array.array->tab + (array.vtable->info.size) * i;
}

void __Array_set_size(__ArrayInfo array, size_t size) {
    if (size > array.array->capacity) {
        __Array_set_capacity(array, max(array.array->size * 2, size));
    }
    else if (size * 2 < array.array->capacity) {
        __Array_set_capacity(array, array.array->capacity / 2);
    }

    array.array->size = size;
}

void __Array_set_capacity(__ArrayInfo array, size_t capacity) {
    array.array->tab = realloc(array.array->tab, array.vtable->info.size * capacity);
    array.array->capacity = capacity;
}
