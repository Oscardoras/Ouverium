#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "data.h"


typedef struct __Array {
    void *tab;
    size_t size;
    size_t capacity;
} __Array;

__Array __UnknownData_get_array(__UnknownData data);


#endif
