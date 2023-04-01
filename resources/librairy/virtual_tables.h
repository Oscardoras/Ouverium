#ifndef __VIRTUAL_TABLES_H__
#define __VIRTUAL_TABLES_H__

#include "data.h"


__Array __VirtualTable_NULL_get_array(__UnknownData data);

void __VirtualTable_Array_gc_iterator(void*);
__Array __VirtualTable_Array_get_array(__UnknownData data);
__UnknownData __VirtualTable_Array_unknown_data_from(void* ptr);
__VirtualTable __VirtualTable_Array = {
    .gc_iterator = __VirtualTable_Array_gc_iterator,
    .get_array = __VirtualTable_Array_get_array,
    .unknown_data_from = __VirtualTable_Array_unknown_data_from,
    .size = sizeof(__Array)
};

__UnknownData __VirtualTable_Int_unknown_data_from(void* ptr);
extern __VirtualTable __VirtualTable_Int;

__UnknownData __VirtualTable_Float_unknown_data_from(void* ptr);
extern __VirtualTable __VirtualTable_Float;

__UnknownData __VirtualTable_Char_unknown_data_from(void* ptr);
extern __VirtualTable __VirtualTable_Char;

__UnknownData __VirtualTable_Bool_unknown_data_from(void* ptr);
extern __VirtualTable __VirtualTable_Bool;


#endif
