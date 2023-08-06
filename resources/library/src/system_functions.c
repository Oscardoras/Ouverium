#include "gc.h"
#include "include.h"


__Reference_Owned __system_function_separator_body(__Reference_Owned capture[], __Reference_Shared args[]) {
    return __Reference_get_element(args[0], __Reference_get_size(args[0])-1);
}

__FunctionCell __system_function_separator = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_separator_body,
    .references = {
        .size = 0,
    }
};

bool __system_function_copy_filter(__Reference_Owned capture[], __Reference_Shared args[]) {
    __VirtualTable* vtable = __Reference_get(args[0]).virtual_table;

    return
        vtable == &__VirtualTable_Int ||
        vtable == &__VirtualTable_Float ||
        vtable == &__VirtualTable_Char ||
        vtable == &__VirtualTable_Bool;
}
__Reference_Owned __system_function_copy_body(__Reference_Owned capture[], __Reference_Shared args[]) {
    __UnknownData data = __Reference_get(args[0]);

    return __Reference_new_data(data);
}

__FunctionCell __system_function_copy = {
    .next = NULL,
    .filter = __system_function_copy_filter,
    .body = __system_function_copy_body,
    .references = {
        .size = 0,
    }
};

__Reference_Owned __system_function_copy_pointer_body(__Reference_Owned capture[], __Reference_Shared args[]) {
    __UnknownData data = __Reference_get(args[0]);

    return __Reference_new_data(data);
}

__FunctionCell __system_function_copy_pointer = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_copy_pointer_body,
    .references = {
        .size = 0,
    }
};

static void assign(__GC_Reference* variable, __UnknownData value) {
    switch (variable->type) {
    case SYMBOL:
        *variable->symbol = value;
        break;
    case PROPERTY: {
        if (variable->property.virtual_table == &__VirtualTable_UnknownData)
            *((__UnknownData*)variable->property.property) = value;
        else if (variable->property.virtual_table == &__VirtualTable_Int)
            *((long*)variable->property.property) = value.data.i;
        else if (variable->property.virtual_table == &__VirtualTable_Float)
            *((double*)variable->property.property) = value.data.f;
        else if (variable->property.virtual_table == &__VirtualTable_Char)
            *((char*)variable->property.property) = value.data.c;
        else if (variable->property.virtual_table == &__VirtualTable_Bool)
            *((bool*)variable->property.property) = value.data.b;
        else
            *((void**)variable->property.property) = value.data.ptr;
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
    }
}
__Reference_Owned __system_function_assign_body(__Reference_Owned capture[], __Reference_Shared args[]) {
    __Reference_Shared var = args[0];
    __Reference_Shared data = args[1];

    assign((__GC_Reference*)var, __Reference_get(data));

    return __Reference_copy(var);
}

__FunctionCell __system_function_assign = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_assign_body,
    .references = {
        .size = 0,
    }
};
