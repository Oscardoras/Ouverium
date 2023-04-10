#include <stdlib.h>

#include "array.h"


size_t max(size_t a, size_t b);
inline size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

void* __Array_get(__ArrayInfo array, unsigned long i) {
    return array.array->tab + (array.vtable->size) * i;
}

void __Array_set_size(__ArrayInfo array, size_t size) {
    if (size > array.array->capacity) {
        __Array_set_capacity(array, max(array.array->size * 2, size));
    } else if (size * 2 < array.array->capacity) {
        __Array_set_capacity(array, array.array->capacity / 2);
    }

    array.array->size = size;
}

void __Array_set_capacity(__ArrayInfo array, size_t capacity) {
    array.array->tab = realloc(array.array->tab ,array.vtable->size * capacity);
    array.array->capacity = capacity;
}
