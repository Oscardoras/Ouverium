#include "system_functions.h"


__Reference __system_function_separator_body(__GC_Context* parent_context, __Reference args) {
    __UnknownData data = __Reference_get(args);
    __Array array = __UnknownData_get_array(data);

    if (array.size > 0) {
        return __Reference {
            .type = DATA,
            .data = data.virtual_table->unknown_data_from(array.tab + data.virtual_table->size * (array.size-1))
        };
    }
}

bool __system_function_copy_filter(__GC_Context* parent_context, __Reference args) {
    __VirtualTable vtable = __Reference_get(args).virtual_table;

    return
        vtable == &__VirtualTable_int_unknown_data_from ||
        vtable== &__VirtualTable_float_unknown_data_from ||
        vtable == &__VirtualTable_char_unknown_data_from ||
        vtable == &__VirtualTable_bool_unknown_data_from;
}
__Reference __system_function_copy_body(__GC_Context* parent_context, __Reference args) {
    __UnknownData data = __Reference_get(args);

    return __Reference {
        .type = DATA,
        .data = data
    };
}

__Reference __system_function_copy_pointer_body(__GC_Context* parent_context, __Reference args) {
    __UnknownData data = __Reference_get(args);

    return __Reference {
        .type = DATA,
        .data = data
    };
}
