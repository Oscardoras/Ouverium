#ifndef __DATA_H__
#define __DATA_H__

#include "array.h"


#ifdef __cplusplus
extern "C" {
#endif

struct __VirtualTable;
struct __UnknownData;

/**
 * Type of a function iterator called by the garbage collector.
*/
typedef void (*__GC_Iterator)(void*);

struct __VirtualTable_Info {
    size_t size;
    __GC_Iterator gc_iterator;
    struct __VirtualTable* array_vtable;
};
union __VirtualTable_TabElement {
    size_t offset;
    void* ptr;
};

/**
 * Contains information to manage a data type.
*/
typedef struct __VirtualTable {
    struct __VirtualTable_Info info;
    union __VirtualTable_TabElement tab[];
} __VirtualTable;

#define __VirtualTable_size(size) \
    struct __VirtualTable ## tab_size { \
        struct __VirtualTable_Info info; \
        union __VirtualTable_TabElement tab[size]; \
    }

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
 * Gets a component from an UnknownData.
 * @param index the index of the component in the virtual tables.
 * @param data the UnknownData.
 * @return a pointer to the component.
*/
void* __UnknownData_get_component_at(size_t index, __UnknownData data);

/**
 * Gets a component from an UnknownData.
 * @param component the name of the component.
 * @param data the UnknownData.
 * @return a pointer to the component.
*/
#define __UnknownData_get_component(component, data) __UnknownData_get_component_at(__Component_ ## component ## _index, data)

/**
 * Gets the array of an UnknownData.
 * @param data an UnknownData.
 * @return an ArrayInfo representing the array of the data.
*/
__ArrayInfo __UnknownData_get_array(__UnknownData data);

#ifdef __cplusplus
}
#endif


#endif
