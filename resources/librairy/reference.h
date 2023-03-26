#ifndef __REFERENCE_H__
#define __REFERENCE_H__

#include "data.h"


/**
 * Represents a reference to a data.
*/
typedef struct __Reference {
    enum {
        DATA,
        SYMBOL,
        PROPERTY,
        ARRAY,
    } type;
    union {
        __UnknownData data;
        __UnknownData* symbol;
        struct {
            void* object;
            __UnknownData* property;
        } property;
        struct {
            __UnknownData array;
            unsigned long i;
        } array;
    };
} __Reference;

__UnknownData __Reference_get(__Reference reference);

__Reference __Reference_break(__Reference reference);


#endif
