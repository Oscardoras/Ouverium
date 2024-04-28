#include <stdlib.h>

#include <ouverium/include.h>


void* Ov_Array_get(Ov_ArrayInfo array, size_t i) {
    return ((BYTE*) array.array->tab) + array.vtable->size * i;
}

void Ov_Array_set_size(Ov_ArrayInfo array, size_t size) {
    if (size > array.array->capacity) {
        size_t d = array.array->size * 2;
        Ov_Array_set_capacity(array, d > size ? d : size);
    } else if (array.array->capacity > 3 && size * 2 < array.array->capacity) {
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
    } else
        if (capacity > 0)
            array.array->tab = malloc(capacity * array.vtable->size);

    array.array->capacity = capacity;
}
