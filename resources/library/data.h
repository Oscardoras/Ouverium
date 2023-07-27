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
        size_t array_size;
        struct __VirtualTable* array_vtable;
    };
    struct __VirtualTable_Element {
        int hash;
        union {
            size_t offset;
            void* ptr;
        };
        struct __VirtualTable_Element* next;
    };

    /**
     * Contains information to manage a data type.
    */
    typedef struct __VirtualTable {
        struct __VirtualTable_Info info;
        struct __VirtualTable_Element tab[];
    } __VirtualTable;

#define __VirtualTable_size(size) \
    struct __VirtualTable ## tab_size { \
        struct __VirtualTable_Info info; \
        union __VirtualTable_Element tab[size]; \
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
     * @param data the UnknownData.
     * @param hash the hash of the property.
     * @return a pointer to the component.
    */
    void* __UnknownData_get_property(__UnknownData data, int hash);

    /**
     * Gets the array of an UnknownData.
     * @param data an UnknownData.
     * @return an ArrayInfo representing the array of the data.
    */
    __ArrayInfo __UnknownData_get_array(__UnknownData data);

    int hash(const char *string);

#ifdef __cplusplus
}
#endif


#endif
