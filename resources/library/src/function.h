#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include "include.h"


#ifdef __cplusplus
extern "C" {
#endif

    typedef struct __FunctionCell {
        struct __FunctionCell* next;
        __FunctionFilter filter;
        __FunctionBody body;
        struct {
            unsigned short size;
            __Reference_Owned* tab;
        } references;
    } __FunctionCell;

#ifdef __cplusplus
}
#endif


#endif
