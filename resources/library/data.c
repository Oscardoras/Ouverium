#include "data.h"
#include "virtual_tables.h"


__UnknownData __UnknownData_get(__VirtualTable* vtable, void* ptr) {
    __UnknownData data;
    data.virtual_table = vtable;

    if (vtable == &__VirtualTable_Int)
        data.data.i = *((int*) ptr);
    else if (vtable == &__VirtualTable_Float)
        data.data.f = *((int*) ptr);
    else if (vtable == &__VirtualTable_Char)
        data.data.c = *((int*) ptr);
    else if (vtable == &__VirtualTable_Bool)
        data.data.b = *((int*) ptr);
    else
        data.data.ptr = *((void**) ptr);

    return data;
}

__ArrayInfo __UnknownData_get_array(__UnknownData data) {
    return data.virtual_table->get_array(data);
}
