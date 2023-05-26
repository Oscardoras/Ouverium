#include "gc.h"
#include "virtual_tables.h"


void __VirtualTable_UnknownData_gc_iterator(void* ptr) {
    __UnknownData data = *((__UnknownData*)ptr);
    if (
        data.virtual_table != &__VirtualTable_Int &&
        data.virtual_table != &__VirtualTable_Float &&
        data.virtual_table != &__VirtualTable_Char &&
        data.virtual_table != &__VirtualTable_Bool
        )
        data.virtual_table->info.gc_iterator(data.data.ptr);
}
__VirtualTable __VirtualTable_UnknownData = {
    .info.size = sizeof(__UnknownData),
    .info.gc_iterator = __VirtualTable_UnknownData_gc_iterator,
    .info.array_vtable = NULL
};

void __VirtualTable_Array_gc_iterator(void* ptr) {
    __ArrayInfo array = __UnknownData_get_array(*((__UnknownData*)ptr));
    size_t i;
    for (i = 0; i < array.array->size; i++)
        __VirtualTable_UnknownData_gc_iterator(__Array_get(array, i));
}
__VirtualTable __VirtualTable_Array = {
    .info.size = sizeof(__Array),
    .info.gc_iterator = __VirtualTable_Array_gc_iterator,
    .info.array_vtable = &__VirtualTable_UnknownData
};

__VirtualTable __VirtualTable_Int = {
    .info.size = sizeof(long),
    .info.gc_iterator = __GC_NULL_iterator,
    .info.array_vtable = &__VirtualTable_UnknownData
};

__VirtualTable __VirtualTable_Float = {
    .info.size = sizeof(double),
    .info.gc_iterator = __GC_NULL_iterator,
    .info.array_vtable = &__VirtualTable_UnknownData
};

__VirtualTable __VirtualTable_Char = {
    .info.size = sizeof(char),
    .info.gc_iterator = __GC_NULL_iterator,
    .info.array_vtable = &__VirtualTable_UnknownData
};

__VirtualTable __VirtualTable_Bool = {
    .info.size = sizeof(bool),
    .info.gc_iterator = __GC_NULL_iterator,
    .info.array_vtable = &__VirtualTable_UnknownData
};
