#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdbool.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

    struct __VirtualTable;

    typedef struct __Array {
        void* tab;
        size_t size;
        size_t capacity;
    } __Array;
#define __Component___Array_index 1

    /**
     * Represents an array and the virtual table of its data.
    */
    typedef struct __ArrayInfo {
        struct __VirtualTable* vtable;
        struct __Array* array;
    } __ArrayInfo;

    /**
     * Creates a new array.
     * @param vtable the vtable of array elements.
     * @param capacity the initial capacity of the array.
     * @return a created array.
    */
    __Array __Array_new(struct __VirtualTable* vtable, size_t capacity);

    /**
     * Gets an element in an array.
     * @param array the array.
     * @param i the index of the element to get.
     * @return a pointer to the element.
    */
    void* __Array_get(__ArrayInfo array, size_t i);

    /**
     * Sets the size of the array, changes the capacity if necessary.
     * @param array the array.
     * @param size the new size.
    */
    void __Array_set_size(__ArrayInfo array, size_t size);

    /**
     * Sets the capacity of the array.
     * @param array the array.
     * @param capacity the new capacity.
    */
    void __Array_set_capacity(__ArrayInfo array, size_t capacity);

#ifdef __cplusplus
}
#endif


#endif
