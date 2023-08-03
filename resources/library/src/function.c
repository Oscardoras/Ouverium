#include <stdlib.h>

#include "include.h"
#include "function.h"


__Function __Function_new() {
    return NULL;
}

void __Function_push(__Function* function, __FunctionBody body, __FunctionFilter filter, __Reference_Owned references[], size_t references_size) {
    __FunctionCell* f = malloc(sizeof(__Function) + references_size * sizeof(__Reference_Owned));

    f->next = *function;
    f->filter = filter;
    f->body = body;
    f->references.size = references_size;
    f->references.tab = (__Reference_Owned*)f + 1;

    size_t i;
    for (i = 0; i < references_size; ++i)
        f->references.tab[i] = references[i];

    *function = f;
}

void __Function_pop(__Function* function) {
    __FunctionCell* f = *((__FunctionCell**)function);
    __FunctionCell* next = f->next;

    size_t i;
    for (i = 0; i < f->references.size; ++i)
        __Reference_free(f->references.tab[i]);

    free(*function);

    *function = next;
}

void __Function_free(__Function* function) {
    while (*function != NULL)
        __Function_pop(function);
}

__Reference_Owned __Function_eval(__Function function, __Reference_Shared args) {
    __FunctionCell* ptr;
    for (ptr = function; ptr != NULL; ptr = ptr->next) {
        if (ptr->filter == NULL || ptr->filter(args)) {
            return ptr->body(args);
        }
    }
}
