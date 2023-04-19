#ifndef __DATA_H__
#define __DATA_H__

#include <stddef.h>

#include "gc.h"


struct __VirtualTable;
struct __UnknownData;
struct __Array;

/**
 * Represents an array and the virtual table of its data.
*/
typedef struct __ArrayInfo {
    struct __VirtualTable* vtable;
    struct __Array* array;
} __ArrayInfo;

/**
 * Type of a function to get an array from a data.
*/
typedef struct __ArrayInfo (*__UnknownData_GetArray)(struct __UnknownData data);

/**
 * Contains information to manage a data type.
*/
typedef struct __VirtualTable {
    __GC_Iterator gc_iterator;
    __UnknownData_GetArray get_array;
    size_t size;
} __VirtualTable;

/**
 * Represents a data which the real type is unknown.
*/
typedef struct __UnknownData {
    __VirtualTable* virtual_table;
    union {
        void* ptr;
        long i;
        double f;
        char c;
        bool b;
    } data;
} __UnknownData;

/**
 * Creates an UnknownData from a virtual table and a data.
 * @param vtable the virtual table of the data.
 * @param d a data.
 * @return an UnknownData.
*/
__UnknownData __UnknownData_from_data(__VirtualTable* vtable, void* d, ...);

/**
 * Creates an UnknownData from a virtual table and a pointer.
 * @param vtable the virtual table of the data.
 * @param ptr a pointer to the data.
 * @return an UnknownData.
*/
__UnknownData __UnknownData_from_ptr(__VirtualTable* vtable, void* ptr);

/**
 * Gets the array of an UnknownData.
 * @param data an UnknownData.
 * @return an ArrayInfo representing the array of the data.
*/
__ArrayInfo __UnknownData_get_array(__UnknownData data);


#endif
