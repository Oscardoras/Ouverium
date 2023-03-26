#ifndef __SYSTEM_FUNCTIONS_H__
#define __SYSTEM_FUNCTIONS_H__

#include <stdlib.h>

#include "function.h"


__Reference __system_function_separator(__GC_Context* parent_context, __Reference args);
__Function separator {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_separator,
    .references {
        .size = 0,
        .tab = NULL
    }
};

__Reference __system_function_copy(__GC_Context* parent_context, __Reference args);
__Function separator {
    .next = NULL,
    .filter = NULL,
    .body = __system_function_copy,
    .references {
        .size = 0,
        .tab = NULL
    }
};


#endif
