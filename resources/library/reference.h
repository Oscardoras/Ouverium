#ifndef __REFERENCE_H__
#define __REFERENCE_H__

#include "array.h"


typedef struct __Reference_Symbol {
    struct __Reference_Symbol* previous;
    struct __UnknownData data;
    unsigned int references;
    struct __Reference_Symbol* next;
} __Reference_Symbol;

/**
 * Represents a reference to a data.
*/
typedef struct __Reference {
    enum {
        DATA,
        SYMBOL,
        PROPERTY,
        ARRAY,
        TUPLE_DYNAMIC,
        TUPLE_STATIC
    } type;
    union {
        __UnknownData data;
        __Reference_Symbol* symbol;
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
 * Gets a tuple reference from a Reference that can be an array.
 * @param reference the Reference.
 * @return a tuple reference.
*/
__Reference __Reference_get_tuple(__Reference reference);

/**
 * Copies a Reference.
 * @param reference the Reference.
 * @return a Reference.
*/
__Reference __Reference_copy(__Reference reference);

/**
 * Frees a reference when it has been used.
 * All references must be freed.
 * @param reference a reference.
*/
void __Reference_free(__Reference reference);

/**
 * Creates a new symbol Reference with a counter initialized to 1.
 * The Reference is destroyed when the counter reaches 0.
 * @return a Reference.
*/
__Reference __Reference_new_symbol();


#endif
