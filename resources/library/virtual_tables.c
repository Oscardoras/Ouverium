#include "virtual_tables.h"


__ArrayInfo __VirtualTable_NULL_get_array(__UnknownData data) {
    __ArrayInfo array = {
        .vtable = NULL,
        .array = NULL
    };
    return array;
}

void __VirtualTable_UnknownData_gc_iterator(void* ptr) {
    __UnknownData data = *((__UnknownData*) ptr);
    if (
        data.virtual_table != &__VirtualTable_Int &&
        data.virtual_table != &__VirtualTable_Float &&
        data.virtual_table != &__VirtualTable_Char &&
        data.virtual_table != &__VirtualTable_Bool
    )
        data.virtual_table->gc_iterator(data.data.ptr);
}
__ArrayInfo __VirtualTable_UnknownData_get_array(__UnknownData data) {
    return data.virtual_table->get_array(data);
}
__VirtualTable __VirtualTable_UnknownData = {
    .gc_iterator = __VirtualTable_UnknownData_gc_iterator,
    .get_array = __VirtualTable_UnknownData_get_array,
    .size = sizeof(__UnknownData)
};

void __VirtualTable_Array_gc_iterator(void* ptr) {
    __ArrayInfo array = {
        .vtable = &__VirtualTable_UnknownData,
        .array = (__Array*) ptr
    };
    for (unsigned long i = 0; i < array.array->size; i++)
        __VirtualTable_UnknownData_gc_iterator(__Array_get(array, i));
}
__ArrayInfo __VirtualTable_Array_get_array(__UnknownData data) {
    __ArrayInfo array = {
        .vtable = &__VirtualTable_UnknownData,
        .array = (__Array*) data.data.ptr
    };
    return array;
}
__VirtualTable __VirtualTable_Array = {
    .gc_iterator = __VirtualTable_Array_gc_iterator,
    .get_array = __VirtualTable_Array_get_array,
    .size = sizeof(__Array)
};

__VirtualTable __VirtualTable_Int = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .size = sizeof(long)
};

__VirtualTable __VirtualTable_Float = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .size = sizeof(double)
};

__VirtualTable __VirtualTable_Char = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .size = sizeof(char)
};

__VirtualTable __VirtualTable_Bool = {
    .gc_iterator = __GC_NULL_iterator,
    .get_array = __VirtualTable_NULL_get_array,
    .size = sizeof(bool)
};
