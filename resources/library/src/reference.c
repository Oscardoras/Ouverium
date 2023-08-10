#include <stdlib.h>
#include <string.h>

#include "include.h"
#include "gc.h"


void __Reference_new_data(__Reference* reference, __UnknownData data) {
    reference->type = DATA;
    reference->data = data;

    __GC_add_reference(reference);
}

void __Reference_new_symbol(__Reference* reference, __UnknownData data) {
    reference->type = SYMBOL;
    reference->symbol = malloc(__VirtualTable_UnknownData.size);

    *reference->symbol = data;

    __GC_add_reference(reference);
}

void __Reference_new_property(__Reference* reference, __UnknownData parent, __VirtualTable* virtual_table, uint32_t hash) {
    reference->type = PROPERTY;
    reference->property.parent = parent;
    reference->property.virtual_table = virtual_table;
    reference->property.hash = hash;

    __GC_add_reference(reference);
}

void __Reference_new_array(__Reference* reference, __UnknownData array, size_t i) {
    reference->type = PROPERTY;
    reference->array.array = array;
    reference->array.i = i;

    __GC_add_reference(reference);
}

void __Reference_new_tuple(__Reference* reference, __Reference references[], size_t references_size) {
    reference->type = TUPLE;
    reference->tuple.heap_allocation = false;
    reference->tuple.size = references_size;
    reference->tuple.tab = references;

    __GC_add_reference(reference);
}

void __Reference_new_from(__Reference* reference, __Reference old) {
    *reference = old;

    __GC_add_reference(reference);
}

__UnknownData __Reference_get(__Reference reference) {
    switch (reference.type) {
    case DATA:
        return reference.data;
    case SYMBOL:
        return *reference.symbol;
    case PROPERTY:
        return __UnknownData_from_ptr(reference.property.virtual_table, __UnknownData_get_property(reference.property.parent, reference.property.hash));
    case ARRAY: {
        __ArrayInfo array = __UnknownData_get_array(reference.array.array);
        return  __UnknownData_from_ptr(array.vtable, __Array_get(array, reference.array.i));
    }
    case TUPLE: {
        __ArrayInfo array = {
            .vtable = &__VirtualTable_UnknownData,
            .array = __GC_alloc_object(&__VirtualTable_Array)
        };
        __Array_set_capacity(array, reference.tuple.size);

        __UnknownData data = {
            .virtual_table = &__VirtualTable_Array,
            .data.ptr = array.array
        };
        return data;
    }
    default: {
        __UnknownData data = {
            .virtual_table = NULL,
            .data.ptr = NULL
        };
        return data;
    }
    }
}

__Reference __Reference_get_element(__Reference reference, size_t i) {
    if (reference.type == TUPLE) {
        return reference.tuple.tab[i];
    }
    else {
        __UnknownData data = __Reference_get(reference);
        __Reference r;
        __Reference_new_array(&r, data, i);
        return r;
    }
}

size_t __Reference_get_size(__Reference reference) {
    if (reference.type == TUPLE) {
        return reference.tuple.size;
    }
    else {
        __UnknownData data = __Reference_get(reference);
        __ArrayInfo array = __UnknownData_get_array(data);
        return array.array->size;
    }
}

void __Reference_move_data(__Reference* dest, __Reference* src) {
    *dest = *src;

    if (src->type == TUPLE && !src->tuple.heap_allocation) {
        dest->tuple.tab = malloc(sizeof(__Reference) * src->tuple.size);
        size_t i;
        for (i = 0; i < src->tuple.size; ++i)
            __Reference_move_data(&dest->tuple.tab[i], &src->tuple.tab[i]);
    }
}

__Reference __Reference_move(__Reference* reference) {
    __Reference r;
    __Reference_move_data(&r, reference);
    __GC_move_reference(reference);
    return r;
}

void __Reference_free_data(__Reference* reference) {
    if (reference->type == TUPLE && reference->tuple.heap_allocation) {
        size_t i;
        for (i = 0; i < reference->tuple.size; ++i)
            __Reference_free_data(&reference->tuple.tab[i]);
        free(reference->tuple.tab);
    }
}

void __Reference_free(__Reference* reference) {
    __GC_remove_reference(reference);
    __Reference_free_data(reference);
}
