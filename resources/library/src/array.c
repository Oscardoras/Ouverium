#include <stdlib.h>

#include "include.h"


size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

__Array __Array_new(__VirtualTable* vtable, size_t capacity) {
    __Array array = {
        .tab = capacity > 0 ? calloc(capacity, vtable->size) : NULL,
        .size = 0,
        .capacity = capacity
    };

    return array;
}

void* __Array_get(__ArrayInfo array, size_t i) {
    return ((BYTE*) array.array->tab) + (array.vtable->size) * i;
}

void __Array_set_size(__ArrayInfo array, size_t size) {
    if (size > array.array->capacity) {
        __Array_set_capacity(array, max(array.array->size * 2, size));
    }
    else if (array.array->capacity > 3 && size * 2 < array.array->capacity) {
        __Array_set_capacity(array, array.array->capacity / 2);
    }

    array.array->size = size;
}

void __Array_set_capacity(__ArrayInfo array, size_t capacity) {
    if (array.array->tab != NULL) {
        if (capacity > 0)
            array.array->tab = realloc(array.array->tab, array.vtable->size * capacity);
        else {
            free(array.array->tab);
            array.array->tab = NULL;
        }
    }
    else
        array.array->tab = calloc(capacity, array.vtable->size);
    array.array->capacity = capacity;
}

void __Array_free(__ArrayInfo array) {
    free(array.array->tab);
}

void __VirtualTable_Array_gc_iterator(void* ptr) {
    __ArrayInfo array = __UnknownData_get_array(*((__UnknownData*)ptr));
    size_t i;
    for (i = 0; i < array.array->size; i++)
        __VirtualTable_UnknownData.gc_iterator(__Array_get(array, i));
}
void __VirtualTable_Array_gc_destructor(void* ptr) {
    __ArrayInfo array = __UnknownData_get_array(*((__UnknownData*)ptr));
    __Array_free(array);
}
__VirtualTable __VirtualTable_Array = {
    .size = sizeof(__Array),
    .gc_iterator = __VirtualTable_Array_gc_iterator,
    .gc_destructor = __VirtualTable_Array_gc_destructor,
    .array.vtable = &__VirtualTable_UnknownData,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};
