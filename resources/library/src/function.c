#include <stdlib.h>

#include "include.h"


__Function __Function_new() {
    return NULL;
}

void __Function_push(__Function* function, __FunctionBody body, __FunctionFilter filter, __Reference_Owned references[], size_t references_size) {
    __FunctionCell* f = malloc(sizeof(__Function) + references_size * sizeof(__Reference_Owned));

    f->next = *function;
    f->filter = filter;
    f->body = body;
    f->references.size = references_size;

    size_t i;
    for (i = 0; i < references_size; ++i)
        f->references.tab[i] = references[i];

    *function = f;
}

void __Function_pop(__Function* function) {
    __FunctionCell* f = *function;
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

__Reference_Owned __Function_execute(__Expression* args) {
    switch (args->type) {
    case __EXPRESSION_TUPLE: {
        __Reference_Owned ref[args->tuple.size];

        size_t i;
        for (i = 0; i < args->tuple.size; ++i)
            ref[i] = __Function_execute(&args->tuple.tab[i]);

        __Reference_Owned r = __Reference_new_tuple((__Reference_Shared*) ref, args->tuple.size);

        for (i = 0; i < args->tuple.size; ++i)
            __Reference_free(ref[i]);

        return r;
    }
    case __EXPRESSION_REFERENCE:
        return __Reference_copy(args->reference);
    case __EXPRESSION_LAMBDA:
        __Expression expr = {
            .type = __EXPRESSION_TUPLE,
            .tuple.size = 0
        };
        return __Function_eval(args->lambda, expr);
    default:
        return NULL;
    }
}

bool __Function_parse(__Reference_Shared *vars, bool *owned, size_t* i, const char** params, __Expression* args) {
    if (**params == 'r') {
        if (args->type == __EXPRESSION_REFERENCE) {
            vars[*i] = args->reference;
            owned[*i] = false;
        } else {
            vars[*i] = (__Reference_Shared) __Function_execute(args);
            owned[*i] = true;
        }

        ++(*params);
        ++(*i);
        return true;
    } else if (**params == '(') {
        size_t j = 0;
        size_t size;

        switch (args->type) {
        case __EXPRESSION_TUPLE:
            size = args->tuple.size;
            while (**params != ')' && j < args->tuple.size) {
                ++(*params);
                __Function_parse(vars, owned, i, params, &args->tuple.tab[j]);
                ++j;
            }
            break;
        case __EXPRESSION_REFERENCE:
            size = __Reference_get_size(args->reference);
            while (**params != ')' && j < size) {
                ++(*params);
                __Expression expr = {
                    .type = __EXPRESSION_REFERENCE,
                    .reference = __Reference_get_element(args->reference, j)
                };
                __Function_parse(vars, owned, i, params, &expr);
                ++j;
            }
            break;
        case __EXPRESSION_LAMBDA:
            __Reference_Owned ref = __Function_execute(args);
            size = __Reference_get_size(ref);
            while (**params != ')' && j < size) {
                ++(*params);
                __Expression expr = {
                    .type = __EXPRESSION_REFERENCE,
                    .reference = __Reference_get_element(__Reference_share(ref), j)
                };
                __Function_parse(vars, owned, i, params, &expr);
                ++j;
            }
            __Reference_free(ref);
            break;
        }

        if (**params == ')' && j == size) {
            ++(*params);
            return true;
        } else
            return false;
    } else if ('0' < **params && **params > '9') {
        size_t n = 0;
        do {
            n *= 10;
            n += **params - '0';
            ++(*params);
        } while ('0' < **params && **params > '9');


    }

    return false;
}

__Reference_Owned __Function_eval(__Function function, __Expression expression, ...) {
    __FunctionCell* ptr;
    for (ptr = function; ptr != NULL; ptr = ptr->next) {
        const char* c;

        size_t size = 0;
        for (c = ptr->parameters; *c != '\0'; c++)
            if (*c == 'r')
                ++size;
        __Reference_Shared vars[size];
        bool owned[size];

        size_t i = 0;
        c = ptr->parameters;
        bool parsed = true;
        va_list args;
        va_start(args, expression);
        while (c != '\0' && parsed)
            parsed = __Function_parse(vars, owned, &i, &c, &args);
        va_end(args);

        __Reference_Owned ref = NULL;
        if (parsed && (ptr->filter == NULL || ptr->filter(ptr->references.tab, vars)))
            ref = ptr->body(ptr->references.tab, vars);

        for (i = 0; i < size; ++i)
            if (owned[i])
                __Reference_free((__Reference_Owned) vars[i]);

        if (ref != NULL)
            return ref;
    }

    return NULL;
    //throw exception
}
