#include "virtual_tables.h"


__UnkownData __VirtualTable_int_unknown_data_from(void* ptr) {
    return __UnkownData {
        .virtual_table = &__VirtualTable_int,
        .data.i = *((long*) ptr)
    };
}

__UnkownData __VirtualTable_float_unknown_data_from(void* ptr) {
    return __UnkownData {
        .virtual_table = &__VirtualTable_float,
        .data.f = *((double*) ptr)
    };
}

__UnkownData __VirtualTable_char_unknown_data_from(void* ptr) {
    return __UnkownData {
        .virtual_table = &__VirtualTable_char,
        .data.f = *((char*) ptr)
    };
}

__UnkownData __VirtualTable_bool_unknown_data_from(void* ptr) {
    return __UnkownData {
        .virtual_table = &__VirtualTable_bool,
        .data.f = *((bool*) ptr)
    };
}
