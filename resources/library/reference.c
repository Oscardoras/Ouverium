#include <stdarg.h>
#include <string.h>

#include "gc.h"
#include "virtual_tables.h"


__Reference_Owned __Reference_new_data(__UnknownData data) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = DATA;
    reference->data = data;

    return (__Reference_Owned) reference;
}

__Reference_Owned __Reference_new_symbol() {
    __UnknownData* data = __GC_alloc_object(sizeof(__UnknownData));

    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = SYMBOL;
    reference->symbol = data;

    return (__Reference_Owned) reference;
}

__Reference_Owned __Reference_new_property(__UnknownData parent, __VirtualTable* virtual_table, void* property) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = PROPERTY;
    reference->property.parent = parent;
    reference->property.virtual_table = virtual_table;
    reference->property.property = property;

    return (__Reference_Owned) reference;
}

__Reference_Owned __Reference_new_array(__UnknownData array, size_t i) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = PROPERTY;
    reference->array.array = array;
    reference->array.i = i;

    return (__Reference_Owned) reference;
}

__Reference_Owned __Reference_new_tuple(size_t size, __Reference_Shared references, ...) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = TUPLE;
    reference->tuple.references = __GC_alloc_references(size);
    reference->tuple.size = size;

    va_list args;
    va_start(args, references);
    for (size_t i = 0; i < size; i++)
        reference->tuple.references[i] = *((__GC_Reference*) va_arg(args, __Reference_Shared*));
    va_end(args);

    return (__Reference_Owned) reference;
}

__UnknownData __Reference_get(__Reference_Shared r) {
    __GC_Reference* reference = (__GC_Reference*) r;

    switch (reference->type) {
        case DATA:
            return reference->data;
        case SYMBOL:
            return *reference->symbol;
        case PROPERTY:
            return __UnknownData_from_ptr(reference->property.virtual_table, reference->property.property);
        case ARRAY: {
            __ArrayInfo array = __UnknownData_get_array(reference->array.array);
            void* ptr = __Array_get(array, reference->array.i);
            return  __UnknownData_from_ptr(array.vtable, __Array_get(array, reference->array.i));
        }
        case TUPLE: {
            __ArrayInfo array = {
                .vtable = &__VirtualTable_UnknownData,
                .array = __GC_alloc_object(sizeof(__Array))
            };
            __Array_set_capacity(array, reference->tuple.size);

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

__Reference_Owned __Reference_get_element(__Reference_Shared r, size_t i) {
    __GC_Reference* reference = (__GC_Reference*) r;

    if (reference->type == TUPLE) {
        return __Reference_copy(__Reference_share(&reference->tuple.references[i]));
    } else {
        __UnknownData data = __Reference_get(__Reference_share(reference));
        return __Reference_new_array(data, i);
    }
}

size_t __Reference_get_size(__Reference_Shared r) {
    __GC_Reference* reference = (__GC_Reference*) r;

    if (reference->type == TUPLE) {
        return reference->tuple.size;
    } else {
        __UnknownData data = __Reference_get(__Reference_share(reference));
        __ArrayInfo array = __UnknownData_get_array(data);
        return array.array->size;
    }
}

__Reference_Owned __Reference_copy(__Reference_Shared r) {
    __GC_Reference* reference = (__GC_Reference*) r;

    __GC_Reference* new_reference = __GC_alloc_references(1);
    *new_reference = *reference;

    if (new_reference->type == TUPLE) {
        new_reference->tuple.references = __GC_alloc_references(new_reference->tuple.size);
        memcpy(new_reference->tuple.references, reference->tuple.references, sizeof(__GC_Reference) * new_reference->tuple.size);
    }

    return (__Reference_Owned) new_reference;
}

void __Reference_free(__Reference_Owned r) {
    __GC_Reference* reference = (__GC_Reference*) r;

    if (reference->type == TUPLE) {
        for (unsigned short i = 0; i < reference->tuple.size; i++)
            __Reference_free((__Reference_Owned) reference->tuple.references + i);
    }

    __GC_free_reference(reference);
}
