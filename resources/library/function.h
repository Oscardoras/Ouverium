#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stddef.h>

#include "reference.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*__FunctionFilter)(__Reference_Shared args);
typedef __Reference_Owned (*__FunctionBody)(__Reference_Shared args);

/**
 * Represents a function in a function stack.
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

/**
 * Represents a function stack.
*/
typedef __Function* __Function_Stack;
#define __Component___Function_Stack_index 0


__Function_Stack __Function_new();

void __Function_push(__Function_Stack* function, __FunctionBody body, __FunctionFilter filter, __Reference_Owned references[], size_t references_size);
void __Function_pop(__Function_Stack* function);

__Reference_Owned __Function_eval(__Function_Stack function, __Reference_Shared args);

void __Function_free(__Function_Stack* function);

#ifdef __cplusplus
}
#endif


#endif
