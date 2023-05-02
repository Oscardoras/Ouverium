#include <stdlib.h>
#include <string.h>

#include "virtual_tables.h"


_Thread_local __Reference_Symbol* __Reference_symbols = NULL;

__UnknownData __Reference_get(__Reference reference) {
    switch (reference.type) {
        case DATA:
            return reference.data;
        case SYMBOL:
            return reference.symbol->data;
        case PROPERTY:
            return *reference.property.property;
        case ARRAY: {
            __ArrayInfo array = __UnknownData_get_array(reference.array.array);
            void* ptr = __Array_get(array, reference.array.i);
            return  __UnknownData_from_ptr(array.vtable, __Array_get(array, reference.array.i));
        }
        case TUPLE_DYNAMIC:
        case TUPLE_STATIC:
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
    if (reference.type == TUPLE_DYNAMIC || reference.type == TUPLE_STATIC) {
        return reference;
    } else {
        __UnknownData data = __Reference_get(reference);
        __ArrayInfo array = __UnknownData_get_array(data);

        __Reference tuple = {
            .type = TUPLE_DYNAMIC,
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

        __Reference_free(reference);
        return tuple;
    }
}

__Reference __Reference_copy(__Reference reference) {
    __Reference new_reference = reference;

    if (reference.type == SYMBOL)
        new_reference.symbol->references++;
    else if (reference.type == TUPLE_DYNAMIC) {
        new_reference.tuple.references = malloc(sizeof(__Reference) * reference.tuple.size);
        memcpy(new_reference.tuple.references, reference.tuple.size, sizeof(__Reference) * reference.tuple.size);
    }

    return new_reference;
}

void __Reference_free(__Reference reference) {
    if (reference.type == SYMBOL) {
        reference.symbol->references--;
        if (reference.symbol->references == 0) {
            if (reference.symbol->previous != NULL)
                reference.symbol->previous->next = reference.symbol->next;
            else
                __Reference_symbols = reference.symbol->next;

            if (reference.symbol->next != NULL)
                reference.symbol->next->previous = reference.symbol->previous;

            free(reference.symbol);
        }
    } else if (reference.type == TUPLE_DYNAMIC) {
        for (unsigned long i = 0; i < reference.tuple.size; i++)
            __Reference_free(reference.tuple.references[i]);
        free(reference.tuple.references);
    }
}

__Reference __Reference_new_symbol() {
    __Reference_Symbol* symbol = malloc(sizeof(__Reference_Symbol));
    __Reference_symbols->previous = symbol;
    symbol->previous = NULL;
    symbol->references = 1;
    symbol->next = __Reference_symbols;
    __Reference_symbols = symbol;

    __Reference reference = {
        .type = SYMBOL,
        .symbol = symbol
    };
    return reference;
}
