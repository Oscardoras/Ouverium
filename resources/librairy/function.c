#include <stdarg.h>
#include <stdlib.h>

#include "function.h"


__Reference __GC_eval_function(__Function* function, __GC_Context* parent_context, __Reference args) {
    for (__Function* ptr = function; ptr != NULL; ptr = ptr->next) {
        if (function->filter == NULL || function->filter(parent_context, args)) {
            function->body(parent_context, args);
            break;
        }
    }
}
