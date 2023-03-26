#include "system_functions.h"


__Reference __system_function_separator(__GC_Context* parent_context, __Reference args) {
    __GC_Context context {
        .next = NULL,
        .iterator = __GC_NULL_iterator
    };
    parent_context->next = &context;

    __UnknownData data = __Reference_get(args);
    __Array array = __UnknownData_get_array(data);
    if (array.size > 0) {
        return __Reference {
            .type = DATA,
            .data = data.virtual_table->unknown_data_from(array.tab + data.virtual_table->size * (array.size-1))
        };
    }
}

__Reference __system_function_copy(__GC_Context* parent_context, __Reference args) {
    __GC_Context context {
        .next = NULL,
        .iterator = __GC_NULL_iterator
    };
    parent_context->next = &context;

    __UnknownData data = __Reference_get(args);
    switch (data.virtual_table) {
        case &__VirtualTable_int_unknown_data_from:
            return __Reference {
                .type = DATA,
                .data.i = data.i
            };
        case SYMBOL:
            return *reference.symbol;
        case PROPERTY:
            return reference.property.property;
        case ARRAY:
            return reference.array.array.virtual_table->array_iterator(reference.array.array.data.ptr, reference.array.i);
        default:
            break;
    }

    if (array.size > 0) {
        return __Reference {
            .type = DATA,
            .data = data.virtual_table->unknown_data_from(array.tab + data.virtual_table->size * (array.size-1))
        };
    }
}
