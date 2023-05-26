#include <stdarg.h>

#include "data.h"
#include "virtual_tables.h"


__UnknownData __UnknownData_from_data(__VirtualTable* vtable, void* d, ...) {
    __UnknownData data;
    data.virtual_table = vtable;

    va_list args;
    va_start(args, d);

    if (vtable == &__VirtualTable_Int)
        data.data.i = va_arg(args, long);
    else if (vtable == &__VirtualTable_Float)
        data.data.f = va_arg(args, double);
    else if (vtable == &__VirtualTable_Char)
        data.data.c = va_arg(args, int);
    else if (vtable == &__VirtualTable_Bool)
        data.data.b = va_arg(args, int);
    else
        data.data.ptr = va_arg(args, void*);

    va_end(args);

    return data;
}

__UnknownData __UnknownData_from_ptr(__VirtualTable* vtable, void* ptr) {
    __UnknownData data;
    data.virtual_table = vtable;

    if (vtable == &__VirtualTable_Int)
        data.data.i = *((long*) ptr);
    else if (vtable == &__VirtualTable_Float)
        data.data.f = *((double*) ptr);
    else if (vtable == &__VirtualTable_Char)
        data.data.c = *((char*) ptr);
    else if (vtable == &__VirtualTable_Bool)
        data.data.b = *((bool*) ptr);
    else
        data.data.ptr = *((void**) ptr);

    return data;
}

void* __UnknownData_get_component_at(size_t index, __UnknownData data) {
    return ((char*) data.data.ptr) + data.virtual_table->tab[index].offset;
}

__ArrayInfo __UnknownData_get_array(__UnknownData data) {
    __ArrayInfo array = {
        .vtable = data.virtual_table->info.array_vtable,
        .array = __UnknownData_get_component(__Array, data)
    };
    return array;
}
