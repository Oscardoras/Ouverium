#include "virtual_tables.h"


__Array __VirtualTable_NULL_get_array(__UnknownData data) {
    __Array array = {
        .tab = NULL,
        .size = 0,
        .capacity = 0
    };
    return array;
}

void __VirtualTable_Array_gc_iterator(void* ptr) {
    __Array* array = (__Array*) ptr;
    for (size_t i = 0; i < array->size; i++) {

    }
}
__Array __VirtualTable_Array_get_array(__UnknownData data) {
    return *((__Array*) data.data.ptr);
}
__UnknownData __VirtualTable_Array_unknown_data_from(void* ptr) {
    return *((__UnknownData*) ptr);
}

__VirtualTable __VirtualTable_Int = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .unknown_data_from = __VirtualTable_Int_unknown_data_from,
    .size = sizeof(long)
};
__UnknownData __VirtualTable_Int_unknown_data_from(void* ptr) {
    __UnknownData data = {
        .virtual_table = &__VirtualTable_Int,
        .data.i = *((long*) ptr)
    };
    return data;
}

__VirtualTable __VirtualTable_Float = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .unknown_data_from = __VirtualTable_Float_unknown_data_from,
    .size = sizeof(double)
};
__UnknownData __VirtualTable_Float_unknown_data_from(void* ptr) {
    __UnknownData data = {
        .virtual_table = &__VirtualTable_Float,
        .data.f = *((double*) ptr)
    };
    return data;
}

__VirtualTable __VirtualTable_Char = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .unknown_data_from = __VirtualTable_Char_unknown_data_from,
    .size = sizeof(char)
};
__UnknownData __VirtualTable_Char_unknown_data_from(void* ptr) {
    __UnknownData data = {
        .virtual_table = &__VirtualTable_Char,
        .data.f = *((char*) ptr)
    };
    return data;
}

__VirtualTable __VirtualTable_Bool = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .unknown_data_from = __VirtualTable_Bool_unknown_data_from,
    .size = sizeof(bool)
};
__UnknownData __VirtualTable_Bool_unknown_data_from(void* ptr) {
    __UnknownData data = {
        .virtual_table = &__VirtualTable_Bool,
        .data.f = *((bool*) ptr)
    };
    return data;
}
