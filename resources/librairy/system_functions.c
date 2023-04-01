#include "system_functions.h"
#include "virtual_tables.h"


__Reference __system_function_separator_body(__GC_Context* parent_context, __Reference args) {
    __UnknownData data = __Reference_get(args);
    __Array array = __UnknownData_get_array(data);

    if (array.size > 0) {
        __Reference reference = {
            .type = DATA,
            .data = data.virtual_table->unknown_data_from(array.tab + data.virtual_table->size * (array.size-1))
        };
        return reference;
    }
}

bool __system_function_copy_filter(__GC_Context* parent_context, __Reference args) {
    __VirtualTable* vtable = __Reference_get(args).virtual_table;

    return
        vtable == &__VirtualTable_Int ||
        vtable == &__VirtualTable_Float ||
        vtable == &__VirtualTable_Char ||
        vtable == &__VirtualTable_Bool;
}
__Reference __system_function_copy_body(__GC_Context* parent_context, __Reference args) {
    __UnknownData data = __Reference_get(args);

    __Reference reference = {
        .type = DATA,
        .data = data
    };
    return reference;
}

__Reference __system_function_copy_pointer_body(__GC_Context* parent_context, __Reference args) {
    __UnknownData data = __Reference_get(args);

    __Reference reference = {
        .type = DATA,
        .data = data
    };
    return reference;
}
