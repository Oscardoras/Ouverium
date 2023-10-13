#include "include.h"


__UnknownData __UnknownData_from_data(__VirtualTable* vtable, union __Data d) {
    __UnknownData data = {
        .virtual_table = vtable,
        .data = d
    };

    return data;
}

__UnknownData __UnknownData_from_ptr(__VirtualTable* vtable, void* ptr) {
    __UnknownData data;
    data.virtual_table = vtable;

    if (vtable == &__VirtualTable_Int)
        data.data.i = *((long*)ptr);
    else if (vtable == &__VirtualTable_Float)
        data.data.f = *((double*)ptr);
    else if (vtable == &__VirtualTable_Char)
        data.data.c = *((char*)ptr);
    else if (vtable == &__VirtualTable_Bool)
        data.data.b = *((bool*)ptr);
    else
        data.data.ptr = *((void**)ptr);

    return data;
}

void* __UnknownData_get_property(__UnknownData data, unsigned int hash) {
    struct __VirtualTable_Element* list = data.virtual_table->table.tab[hash % data.virtual_table->table.size];
    for (; list->hash != hash; list = list->next);
    return ((char*)data.data.ptr) + list->offset;
}

__ArrayInfo __UnknownData_get_array(__UnknownData data) {
    __ArrayInfo array = {
        .vtable = data.virtual_table->array.vtable,
        .array = (__Array*) ((BYTE*) data.data.ptr) + data.virtual_table->array.offset
    };
    return array;
}

__Function* __UnknownData_get_function(__UnknownData data) {
    return (__Function*) ((BYTE*) data.data.ptr) + data.virtual_table->function.offset;
}

void __VirtualTable_UnknownData_gc_iterator(void* ptr) {
    __UnknownData data = *((__UnknownData*)ptr);
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
    .gc_destructor = NULL,
    .array.vtable = NULL,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};

__VirtualTable __VirtualTable_Int = {
    .size = sizeof(long),
    .gc_iterator = NULL,
    .gc_destructor = NULL,
    .array.vtable = NULL,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};

__VirtualTable __VirtualTable_Float = {
    .size = sizeof(double),
    .gc_iterator = NULL,
    .gc_destructor = NULL,
    .array.vtable = NULL,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};

__VirtualTable __VirtualTable_Char = {
    .size = sizeof(char),
    .gc_iterator = NULL,
    .gc_destructor = NULL,
    .array.vtable = NULL,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};

__VirtualTable __VirtualTable_Bool = {
    .size = sizeof(bool),
    .gc_iterator = NULL,
    .gc_destructor = NULL,
    .array.vtable = NULL,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};
