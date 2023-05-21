#ifndef __VIRTUAL_TABLES_H__
#define __VIRTUAL_TABLES_H__

#include "reference.h"


#ifdef __cplusplus
extern "C" {
#endif


void __VirtualTable_UnknownData_gc_iterator(void*);
extern __VirtualTable __VirtualTable_UnknownData;

void __VirtualTable_Array_gc_iterator(void*);
extern __VirtualTable __VirtualTable_Array;

extern __VirtualTable __VirtualTable_Int;

extern __VirtualTable __VirtualTable_Float;

extern __VirtualTable __VirtualTable_Char;

extern __VirtualTable __VirtualTable_Bool;

#ifdef __cplusplus
}
#endif


#endif
