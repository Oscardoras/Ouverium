#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stddef.h>

#include "reference.h"


typedef bool (*__FunctionFilter)(__GC_Context* parent_context, __Reference args);
typedef __Reference (*__FunctionBody)(__GC_Context* parent_context, __Reference args);

/**
 * Represents a function in a function linked list.
*/
typedef struct __Function {
    struct __Function* next;
    __FunctionFilter filter;
    __FunctionBody body;
    struct {
        unsigned short size;
        __Reference* tab;
    } references;
} __Function;


__Reference __Function_eval(__Function* function, __GC_Context* parent_context, __Reference args);


#endif
