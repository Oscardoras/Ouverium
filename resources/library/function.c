#include <stdlib.h>

#include "function.h"


__Function_Stack __Function_new() {
    return NULL;
}

void __Function_push(__Function_Stack* function, __FunctionBody body, __FunctionFilter filter, __Reference_Owned references[], size_t references_size) {
    __Function* f = malloc(sizeof(__Function) + references_size * sizeof(__Reference_Owned));

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

void __Function_pop(__Function_Stack* function) {
    __Function* next = (*function)->next;

    size_t i;
    for (i = 0; i < (*function)->references.size; ++i)
        __Reference_free((*function)->references.tab[i]);

    free(*function);

    *function = next;
}

__Reference_Owned __Function_eval(__Function_Stack function, __Reference_Shared args) {
    __Function* ptr;
    for (ptr = function; ptr != NULL; ptr = ptr->next) {
        if (function->filter == NULL || function->filter(args)) {
            function->body(args);
            break;
        }
    }
}

void __Function_free(__Function_Stack* function) {
    while (*function != NULL)
        __Function_pop(function);
}
