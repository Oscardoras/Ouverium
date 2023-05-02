#include <string.h>

#include "system_functions.h"
#include "virtual_tables.h"


__Reference __system_function_separator_body(__Reference args) {
    __Reference tuple = __Reference_get_tuple(args);

    __Reference reference = tuple.tuple.references[tuple.tuple.size-1];

    __Reference_free(tuple);
    return reference;
}

bool __system_function_copy_filter(__Reference args) {
    __VirtualTable* vtable = __Reference_get(args).virtual_table;

    __Reference_free(args);
    return
        vtable == &__VirtualTable_Int ||
        vtable == &__VirtualTable_Float ||
        vtable == &__VirtualTable_Char ||
        vtable == &__VirtualTable_Bool;
}
__Reference __system_function_copy_body(__Reference args) {
    __UnknownData data = __Reference_get(args);

    __Reference_free(args);
    __Reference reference = {
        .type = DATA,
        .data = data
    };
    return reference;
}

__Reference __system_function_copy_pointer_body(__Reference args) {
    __UnknownData data = __Reference_get(args);

    __Reference_free(args);
    __Reference reference = {
        .type = DATA,
        .data = data
    };
    return reference;
}

static void assign(__Reference variable, __UnknownData value) {
    switch (variable.type) {
        case SYMBOL:
            variable.symbol->data = value;
        case PROPERTY:
            *variable.property.property = value;
        case ARRAY:
            __ArrayInfo array = __UnknownData_get_array(variable.array.array);
            void* ptr = __Array_get(array, variable.array.i);
            if (array.vtable == &__VirtualTable_UnknownData)
                *((__UnknownData*) ptr) = value;
            if (array.vtable == &__VirtualTable_Int)
                *((long*) ptr) = value.data.i;
            if (array.vtable == &__VirtualTable_Float)
                *((double*) ptr) = value.data.f;
            if (array.vtable == &__VirtualTable_Char)
                *((char*) ptr) = value.data.c;
            if (array.vtable == &__VirtualTable_Bool)
                *((bool*) ptr) = value.data.b;
        case TUPLE_DYNAMIC:
        case TUPLE_STATIC: {
            __ArrayInfo array = __UnknownData_get_array(value);

            for (unsigned long i = 0; i < variable.tuple.size; i++)
                assign(variable.tuple.references[i], __UnknownData_from_ptr(array.vtable, __Array_get(array, i)));
        }
    }
}
__Reference __system_function_assign_body(__Reference args) {
    __Reference tuple = __Reference_get_tuple(args);
    __Reference var = tuple.tuple.references[0];
    __UnknownData data = __Reference_get(tuple.tuple.references[1]);

    assign(var, data);

    __Reference_free(tuple);
    return var;
}
