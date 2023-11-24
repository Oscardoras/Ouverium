#include <stdio.h>

#include "gc.h"
#include "include.h"


__Reference_Owned __system_function_separator_body(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    return __Reference_get_element(args[0], __Reference_get_size(args[0]) - 1);
}

bool __system_function_copy_filter(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    __VirtualTable* vtable = __Reference_get(args[0]).virtual_table;

    return
        vtable == &__VirtualTable_Int ||
        vtable == &__VirtualTable_Float ||
        vtable == &__VirtualTable_Char ||
        vtable == &__VirtualTable_Bool;
}
__Reference_Owned __system_function_copy_body(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    __UnknownData data = __Reference_get(args[0]);

    return __Reference_new_data(data);
}

__Reference_Owned __system_function_copy_pointer_body(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    __UnknownData data = __Reference_get(args[0]);

    return __Reference_new_data(data);
}

static void assign(__GC_Reference* variable, __UnknownData value) {
    switch (variable->type) {
    case SYMBOL:
        *variable->symbol = value;
        break;
    case PROPERTY: {
        void* property = __UnknownData_get_property(variable->property.parent, variable->property.hash);
        if (variable->property.virtual_table == &__VirtualTable_UnknownData)
            *((__UnknownData*)property) = value;
        else if (variable->property.virtual_table == &__VirtualTable_Int)
            *((long*)property) = value.data.i;
        else if (variable->property.virtual_table == &__VirtualTable_Float)
            *((double*)property) = value.data.f;
        else if (variable->property.virtual_table == &__VirtualTable_Char)
            *((char*)property) = value.data.c;
        else if (variable->property.virtual_table == &__VirtualTable_Bool)
            *((bool*)property) = value.data.b;
        else
            *((void**)property) = value.data.ptr;
        break;
    }
    case ARRAY: {
        __ArrayInfo array = __UnknownData_get_array(variable->array.array);
        void* ptr = __Array_get(array, variable->array.i);
        if (array.vtable == &__VirtualTable_UnknownData)
            *((__UnknownData*)ptr) = value;
        else if (array.vtable == &__VirtualTable_Int)
            *((long*)ptr) = value.data.i;
        else if (array.vtable == &__VirtualTable_Float)
            *((double*)ptr) = value.data.f;
        else if (array.vtable == &__VirtualTable_Char)
            *((char*)ptr) = value.data.c;
        else if (array.vtable == &__VirtualTable_Bool)
            *((bool*)ptr) = value.data.b;
        else
            *((void**)ptr) = value.data.ptr;
        break;
    }
    case TUPLE: {
        __ArrayInfo array = __UnknownData_get_array(value);

        size_t i;
        for (i = 0; i < variable->tuple.size; i++)
            assign(&variable->tuple.references[i], __UnknownData_from_ptr(array.vtable, __Array_get(array, i)));
        break;
    }
    default:
        break;
    }
}

__Reference_Owned __system_function_assign_body(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    __Reference_Shared var = args[0];
    __Reference_Shared data = args[1];

    assign((__GC_Reference*)var, __Reference_get(data));

    return __Reference_copy(var);
}

__Reference_Owned __system_function_function_definition_body(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    __Function function = *__UnknownData_get_function(__Reference_get(args[1]));

    __Reference_Owned references[function->captures.size];
    size_t i;
    for (i = 0; i < function->captures.size; ++i)
        references[i] = __GC_capture_to_reference(function->captures.tab[i]);
    __Function_push(__UnknownData_get_function(__Reference_get(args[0])), function->parameters, function->body, function->filter, references, function->captures.size);

    return __Reference_copy(args[0]);
}

__Reference_Owned __system_function_print_body(__Reference_Owned captures[], __Reference_Shared args[]) {
    (void)(captures);
    __Reference_Shared ref = args[0];

    __UnknownData data = __Reference_get(ref);
    if (data.virtual_table == &__VirtualTable_Int)
        printf("%li", data.data.i);

    return __Reference_new_tuple(NULL, 0);
}
