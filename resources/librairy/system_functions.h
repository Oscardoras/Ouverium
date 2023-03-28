#ifndef __SYSTEM_FUNCTIONS_H__
#define __SYSTEM_FUNCTIONS_H__

#include <stdlib.h>

#include "function.h"


__Reference __system_function_separator_body(__GC_Context* parent_context, __Reference args);
__Function __system_function_separator = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_separator_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

bool __system_function_copy_filter(__GC_Context* parent_context, __Reference args);
__Reference __system_function_copy_body(__GC_Context* parent_context, __Reference args);
__Function __system_function_copy = {
    .next = NULL,
    .filter = __system_function_copy_filter,
    .body = __system_function_copy_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

__Reference __system_function_copy_pointer_body(__GC_Context* parent_context, __Reference args);
__Function __system_function_copy_pointer = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_copy_pointer_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};


#endif
