#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "data.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct __Array {
    void *tab;
    size_t size;
    size_t capacity;
} __Array;


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
