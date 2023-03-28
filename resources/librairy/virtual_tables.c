#include "virtual_tables.h"


__Array __VirtualTable_NULL_get_array(__UnknownData data) {
    return __Array {
        .tab = NULL,
        .size = 0,
        .capacity = 0
    };
}

void __VirtualTable_Array_gc_iterator(void* ptr) {
    __Array* array = (__Array*) ptr;
    for (size_t i = 0; i < array->size; i++) {

    }
}
__Array __VirtualTable_Array_get_array(__UnknownData data) {
    return *((__Array*) data.data.tr);
}
__UnknownData __VirtualTable_Array_unknown_data_from(void* ptr) {
    return *((__UnknownData*) ptr);
}

__UnknownData __VirtualTable_Int_unknown_data_from(void* ptr) {
    return __UnknownData {
        .virtual_table = &__VirtualTable_Int,
        .data.i = *((long*) ptr)
    };
}

__UnknownData __VirtualTable_Float_unknown_data_from(void* ptr) {
    return __UnknownData {
        .virtual_table = &__VirtualTable_Float,
        .data.f = *((double*) ptr)
    };
}

__UnknownData __VirtualTable_Char_unknown_data_from(void* ptr) {
    return __UnknownData {
        .virtual_table = &__VirtualTable_Char,
        .data.f = *((char*) ptr)
    };
}

__UnknownData __VirtualTable_Bool_unknown_data_from(void* ptr) {
    return __UnknownData {
        .virtual_table = &__VirtualTable_Bool,
        .data.f = *((bool*) ptr)
    };
}
