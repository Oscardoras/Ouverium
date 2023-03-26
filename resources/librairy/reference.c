#include "reference.h"


__UnknownData __Reference_get(__Reference reference) {
    switch (reference.type) {
        case DATA:
            return reference.data;
        case SYMBOL:
            return *reference.symbol;
        case PROPERTY:
            return reference.property.property;
        case ARRAY:
            return reference.array.array.virtual_table->array_iterator(reference.array.array.data.ptr, reference.array.i);
        default:
            break;
    }
}

__Reference __Reference_break(__Reference reference) {
    __Reference new_reference;

    new_reference.type = DATA;

    switch (reference.type) {
        case DATA:
            new_reference.data = reference.data;
        case SYMBOL:
            new_reference.data = *reference.symbol;
        case PROPERTY:
            new_reference.data = reference.property.object;
            new_reference.property.property = reference.property.property;
        case ARRAY:
            new_reference.array.array = reference.array.array;
            new_reference.array.i = reference.array.i;
        default:
            break;
    }

    return new_reference;
}
