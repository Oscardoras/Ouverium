#include <stdlib.h>

#include "include.h"


static size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

void* Ov_Array_get(Ov_ArrayInfo array, size_t i) {
    return ((BYTE*) array.array->tab) + array.vtable->size * i;
}

void Ov_Array_set_size(Ov_ArrayInfo array, size_t size) {
    if (size > array.array->capacity) {
        Ov_Array_set_capacity(array, max(array.array->size * 2, size));
    }
    else if (array.array->capacity > 3 && size * 2 < array.array->capacity) {
        Ov_Array_set_capacity(array, array.array->capacity / 2);
    }

    array.array->size = size;
}

void Ov_Array_set_capacity(Ov_ArrayInfo array, size_t capacity) {
    if (array.array->tab != NULL) {
        if (capacity > 0)
            array.array->tab = realloc(array.array->tab, array.vtable->size * capacity);
        else {
            free(array.array->tab);
            array.array->tab = NULL;
        }
    }
    else
        array.array->tab = malloc(capacity * array.vtable->size);

    array.array->capacity = capacity;
}
