#include "reference.h"


__UnknownData __Reference_get(__Reference reference) {
    switch (reference.type) {
        case DATA:
            return reference.data;
        case SYMBOL:
            return *reference.symbol;
        case PROPERTY:
            return *reference.property.property;
        case ARRAY: {
            __Array array = __UnknownData_get_array(reference.array.array);
            return reference.data.virtual_table->unknown_data_from(array.tab + reference.data.virtual_table->array_element_virtual_table->size * (array.size-1));
        }
        case TUPLE:

            return reference.array.array.virtual_table->array_iterator(reference.array.array.data.ptr, reference.array.i);
        default:
            break;
    }
}
