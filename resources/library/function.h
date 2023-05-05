#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stddef.h>

#include "reference.h"


typedef bool (*__FunctionFilter)(__Reference_Shared args);
typedef __Reference_Owned (*__FunctionBody)(__Reference_Shared args);

/**
 * Represents a function in a function linked list.
*/
typedef struct __Function {
    struct __Function* next;
    __FunctionFilter filter;
    __FunctionBody body;
    struct {
        unsigned short size;
        __Reference_Owned* tab;
    } references;
} __Function;


__Reference_Owned __Function_eval(__Function* function, __Reference_Shared args);


#endif
