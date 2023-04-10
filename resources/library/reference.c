#include <stdlib.h>

#include "virtual_tables.h"


__UnknownData __Reference_get(__Reference reference) {
    switch (reference.type) {
        case DATA:
            return reference.data;
        case SYMBOL:
            return *reference.symbol;
        case PROPERTY:
            return *reference.property.property;
        case ARRAY: {
            __ArrayInfo array = __UnknownData_get_array(reference.array.array);
            void* ptr = __Array_get(array, reference.array.i);
            return  __UnknownData_get(array.vtable, ptr);
        }
        case TUPLE:
            __ArrayInfo array = {
                .vtable = &__VirtualTable_UnknownData,
                .array = __GC_alloc_object(sizeof(__Array))
            };
            __Array_set_capacity(array, reference.tuple.size);

            __UnknownData data = {
                .virtual_table = &__VirtualTable_Array,
                .data.ptr = array.array
            };
            return data;
        default:
            break;
    }
}

__Reference __Reference_get_tuple(__Reference reference) {
    if (reference.type == TUPLE) {
        return reference;
    } else {
        __UnknownData data = __Reference_get(reference);
        __ArrayInfo array = __UnknownData_get_array(data);

        __Reference tuple = {
            .type = TUPLE,
            .tuple.size = array.array->size,
            .tuple.references = malloc(sizeof(__Reference) * array.array->size)
        };
        for (unsigned long i = 0; i < array.array->size; i++) {
            __Reference r = {
                .type = ARRAY,
                .array.array = data,
                .array.i = i
            };
            tuple.tuple.references[i] = r;
        }

        return tuple;
    }
}
