#include "gc.h"
#include "virtual_tables.h"


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
__VirtualTable __VirtualTable_UnknownData = {
    .size = sizeof(__UnknownData),
    .gc_iterator = __VirtualTable_UnknownData_gc_iterator,
    .array.vtable = NULL,
    .array.offset = 0,
    .function_offset = 0
};

void __VirtualTable_Array_gc_iterator(void* ptr) {
    __ArrayInfo array = __UnknownData_get_array(*((__UnknownData*) ptr));
    size_t i;
    for (i = 0; i < array.array->size; i++)
        __VirtualTable_UnknownData_gc_iterator(__Array_get(array, i));
}
__VirtualTable __VirtualTable_Array = {
    .size = sizeof(__Array),
    .gc_iterator = __VirtualTable_Array_gc_iterator,
    .array.vtable = &__VirtualTable_UnknownData,
    .array.offset = 0,
    .function_offset = 0
};

__VirtualTable __VirtualTable_Int = {
    .size = sizeof(long),
    .gc_iterator = __GC_NULL_iterator,
    .array.vtable = &__VirtualTable_UnknownData,
    .array.offset = 0,
    .function_offset = 0
};

__VirtualTable __VirtualTable_Float = {
    .size = sizeof(double),
    .gc_iterator = __GC_NULL_iterator,
    .array.vtable = &__VirtualTable_UnknownData,
    .array.offset = 0,
    .function_offset = 0
};

__VirtualTable __VirtualTable_Char = {
    .size = sizeof(char),
    .gc_iterator = __GC_NULL_iterator,
    .array.vtable = &__VirtualTable_UnknownData,
    .array.offset = 0,
    .function_offset = 0
};

__VirtualTable __VirtualTable_Bool = {
    .size = sizeof(bool),
    .gc_iterator = __GC_NULL_iterator,
    .array.vtable = &__VirtualTable_UnknownData,
    .array.offset = 0,
    .function_offset = 0
};
