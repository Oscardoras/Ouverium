#ifndef __DATA_H__
#define __DATA_H__

#include <stddef.h>

#include "gc.h"


struct __VirtualTable;
struct __UnknownData;
struct __Array;

/**
 * Type of a function to get an array from a data.
*/
typedef struct __Array (*__UnknownData_GetArray)(struct __UnknownData data);

/**
 * Type of a function to get an UnknowData from a pointer.
*/
typedef struct __UnknownData (*__UnknownData_From)(void* ptr);

/**
 * Contains information to manage a data type.
*/
typedef struct __VirtualTable {
    __GC_Iterator gc_iterator;
    __UnknownData_GetArray get_array;
    __UnknownData_From unknown_data_from;
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

typedef struct __Array {
    void *tab;
    size_t size;
    size_t capacity;
} __Array;

__Array __UnknownData_get_array(__UnknownData data);


#endif
