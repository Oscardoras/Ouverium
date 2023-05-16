#ifndef __DATA_H__
#define __DATA_H__

#include <stdbool.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

struct __VirtualTable;
struct __UnknownData;
struct __Array;

/**
 * Type of a function iterator called by the garbage collector.
*/
typedef void (*__GC_Iterator)(void*);

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

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

extern __VirtualTable __VirtualTable_Int;
extern __VirtualTable __VirtualTable_Float;
extern __VirtualTable __VirtualTable_Char;
extern __VirtualTable __VirtualTable_Bool;

template<class T>
inline __VirtualTable vtable = {
    .size = sizeof(T)
};

class UnknownData {

protected:

    __UnknownData data;

public:

    UnknownData(__UnknownData const& data):
        data{data} {}

    UnknownData(__VirtualTable* vtable, void* d, ...):
        data{__UnknownData_from_data(vtable, d)} {}

    UnknownData(__VirtualTable* vtable, void* ptr):
        data{__UnknownData_from_ptr(vtable, ptr)} {}

    UnknownData(long i) {
        data.virtual_table = &__VirtualTable_Int;
        data.data.i = i;
    }
    UnknownData(double f) {
        data.virtual_table = &__VirtualTable_Float;
        data.data.f = f;
    }
    UnknownData(char c) {
        data.virtual_table = &__VirtualTable_Char;
        data.data.c = c;
    }
    UnknownData(bool b) {
        data.virtual_table = &__VirtualTable_Bool;
        data.data.b = b;
    }
    template<class T>
    UnknownData(T* ptr) {
        data.virtual_table = &T::vtable;
        data.data.b = ptr;
    }

    operator __UnknownData() const {
        return data;
    }

    operator long() const {
        return data.data.i;
    }
    operator double() const {
        return data.data.f;
    }
    operator char() const {
        return data.data.c;
    }
    operator bool() const {
        return data.data.b;
    }
    operator void*() const {
        return data.data.ptr;
    }

    __VirtualTable* virtual_table() const {
        return data.virtual_table;
    }

};

#endif


#endif
