#include <stdlib.h>

#include "function.h"


__Reference __GC_eval_function(__Function* function, __Reference args) {
    for (__Function* ptr = function; ptr != NULL; ptr = ptr->next) {
        if (function->filter == NULL || function->filter(args)) {
            function->body(args);
            break;
        }
    }
}
