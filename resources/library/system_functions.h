#ifndef __SYSTEM_FUNCTIONS_H__
#define __SYSTEM_FUNCTIONS_H__

#include "function.h"


#ifdef __cplusplus
extern "C" {
#endif

__Reference_Owned __system_function_separator_body(__Reference_Shared args);
__Function __system_function_separator = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_separator_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

bool __system_function_copy_filter(__Reference_Shared args);
__Reference_Owned __system_function_copy_body(__Reference_Shared args);
__Function __system_function_copy = {
    .next = NULL,
    .filter = __system_function_copy_filter,
    .body = __system_function_copy_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

__Reference_Owned __system_function_copy_pointer_body(__Reference_Shared args);
__Function __system_function_copy_pointer = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_copy_pointer_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

__Reference_Owned __system_function_assign_body(__Reference_Shared args);
__Function __system_function_assign = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_assign_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

/*
__Reference __system_function_function_definition_body(__Reference args);
__Function __system_function_function_definition = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_function_definition_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};

__Reference __system_function_equals_body(__Reference args);
__Function __system_function_equals = {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_equals_body,
    .references = {
        .size = 0,
        .tab = NULL
    }
};
*/

#ifdef __cplusplus
}
#endif


#endif
