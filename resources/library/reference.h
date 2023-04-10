#ifndef __REFERENCE_H__
#define __REFERENCE_H__

#include "array.h"


/**
 * Represents a reference to a data.
*/
typedef struct __Reference {
    enum {
        DATA,
        SYMBOL,
        PROPERTY,
        ARRAY,
        TUPLE
    } type;
    union {
        __UnknownData data;
        __UnknownData* symbol;
        struct {
            void* ptr_parent;
            __UnknownData* property;
        } property;
        struct {
            __UnknownData array;
            unsigned long i;
        } array;
        struct {
            struct __Reference *references;
            size_t size;
        } tuple;
    };
} __Reference;

/**
 * Gets the UnknownData referenced by a Reference.
 * @param reference the Reference.
 * @return the UnknownData stored in reference.
*/
__UnknownData __Reference_get(__Reference reference);


/**
 * Gets a tuple reference from a Reference.
 * @param reference the Reference.
 * @return a tuple reference.
*/
__Reference __Reference_get_tuple(__Reference reference);


#endif
