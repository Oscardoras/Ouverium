#include <stdlib.h>

#include "include.h"
#include "gc.h"


__Function __Function_new() {
    return NULL;
}

__Function __Function_copy(__Function function) {
    __Function cpy = NULL;

    __FunctionCell* cell;
    for (cell = function; cell != NULL; cell = cell->next) {
        __FunctionCell* f = malloc(sizeof(__Function) + cell->captures.size * sizeof(__Reference));

        f->next = cpy;
        f->parameters = cell->parameters;
        f->filter = cell->filter;
        f->body = cell->body;
        f->captures.size = cell->captures.size;

        size_t i;
        for (i = 0; i < cell->captures.size; ++i)
            __Reference_move_data(&f->captures.tab[i], &cell->captures.tab[i]);

        cpy = f;
    }

    return cpy;
}

void __Function_push(__Function* function, const char* parameters, __FunctionBody body, __FunctionFilter filter, __Reference captures[], size_t captures_size) {
    __FunctionCell* f = malloc(sizeof(__Function) + captures_size * sizeof(__Reference));

    f->next = *function;
    f->parameters = parameters;
    f->filter = filter;
    f->body = body;
    f->captures.size = captures_size;

    size_t i;
    for (i = 0; i < captures_size; ++i)
        __Reference_move_data(&f->captures.tab[i], &captures[i]);

    *function = f;
}

void __Function_pop(__Function* function) {
    __FunctionCell* f = *function;
    __FunctionCell* next = f->next;

    size_t i;
    for (i = 0; i < f->captures.size; ++i)
        __Reference_free_data(&f->captures.tab[i]);

    free(*function);

    *function = next;
}

void __Function_free(__Function* function) {
    while (*function != NULL)
        __Function_pop(function);
}

__Reference __Function_execute(__Expression args) {
    switch (args.type) {
    case __EXPRESSION_TUPLE: {
        __Reference tab[args.tuple.size];

        size_t i;
        for (i = 0; i < args.tuple.size; ++i)
            __Reference_new_from(&tab[i], __Function_execute(args.tuple.tab[i]));

        __Reference r;
        __Reference_new_tuple(&r, tab, args.tuple.size);

        for (i = 0; i < args.tuple.size; ++i)
            __Reference_free(&tab[i]);

        return __Reference_move(&r);
    }
    case __EXPRESSION_REFERENCE:
        return args.reference;
    case __EXPRESSION_LAMBDA:
        __Expression expr = {
            .type = __EXPRESSION_TUPLE,
            .tuple.size = 0
        };
        return __Function_eval(args.lambda, expr);
    default:
        __Reference r = {
            .type = NONE
        };
        return r;
    }
}

bool __Function_parse(__Reference captures[], __Reference* vars, size_t* i, const char** params, __Expression args) {
    if (**params == 'r') {
        ++(*params);

        if (**params == '(') {
            ++(*params);

            union __Data data = {
                .ptr = __GC_alloc_object(&__VirtualTable_Function)
            };
            *((__Function*)data.ptr) = __Function_copy(args.lambda);

            __Reference_new_data(&vars[*i], __UnknownData_from_data(&__VirtualTable_Function, data));
        }
        else if (**params == '.') {
            ++(*params);

            union __Data data = {
                .ptr = __GC_alloc_object(&__VirtualTable_Function)
            };
            __Reference_new_data(&vars[*i], __UnknownData_from_data(&__VirtualTable_Function, data));

            *((__Function*)data.ptr) = __Function_copy(args.lambda);
        }
        else {
            __Reference_new_from(&vars[*i], __Function_execute(args));
        }

        ++(*i);
        return true;
    }
    else if (**params == '(') {
        size_t j = 0;
        size_t size;

        switch (args.type) {
        case __EXPRESSION_TUPLE:
            size = args.tuple.size;
            while (**params != ')' && j < args.tuple.size) {
                ++(*params);
                __Function_parse(captures, vars, i, params, args.tuple.tab[j]);
                ++j;
            }
            break;
        case __EXPRESSION_REFERENCE:
            size = __Reference_get_size(args.reference);
            while (**params != ')' && j < size) {
                ++(*params);
                __Expression expr = {
                    .type = __EXPRESSION_REFERENCE
                };
                __Reference_new_from(&expr.reference, __Reference_get_element(args.reference, j));
                __Function_parse(captures, vars, i, params, expr);
                __Reference_free(&expr.reference);
                ++j;
            }
            break;
        case __EXPRESSION_LAMBDA:
            __Reference ref;
            __Reference_new_from(&ref, __Function_execute(args));
            size = __Reference_get_size(ref);
            while (**params != ')' && j < size) {
                ++(*params);
                __Expression expr = {
                    .type = __EXPRESSION_REFERENCE
                };
                __Reference_new_from(&expr.reference, __Reference_get_element(args.reference, j));
                __Function_parse(captures, vars, i, params, expr);
                __Reference_free(&expr.reference);
                ++j;
            }
            __Reference_free(&ref);
            break;
        }

        if (**params == ')' && j == size) {
            ++(*params);
            return true;
        }
        else
            return false;
    }
    else if ('0' < **params && **params > '9') {
        size_t n = 0;
        do {
            n *= 10;
            n += **params - '0';
            ++(*params);
        } while ('0' < **params && **params > '9');

        __Function* f = __UnknownData_get_function(__Reference_get(captures[n]));
        __Function_eval(*f, args);
    }

    return false;
}

__Reference __Function_eval(__Function function, __Expression args) {
    __Reference ref = {
        .type = NONE
    };

    __FunctionCell* ptr;
    for (ptr = function; ptr != NULL; ptr = ptr->next) {
        const char* c;

        size_t size = 0;
        for (c = ptr->parameters; *c != '\0'; c++)
            if (*c == 'r')
                ++size;
        __Reference vars[size];

        size_t i = 0;
        c = ptr->parameters;
        bool parsed = __Function_parse(ptr->captures.tab, vars, &i, &c, args);
        if (parsed && (ptr->filter == NULL || ptr->filter(ptr->captures.tab, vars)))
            ptr->body(ptr->captures.tab, vars);

        for (i = 0; i < size; ++i)
            __Reference_free(&vars[i]);

        if (ref.type != NONE)
            return ref;
    }

    return ref;
    //throw exception
}

void __VirtualTable_Function_gc_iterator(void* ptr) {
    __FunctionCell* f;
    for (f = *((__Function*)ptr); f != NULL; f = f->next) {
        size_t i;
        for (i = 0; i < f->captures.size; ++i) {
            __UnknownData data = __Reference_get(f->captures.tab[i]);
            __VirtualTable_UnknownData.gc_iterator(&data);
        }
    }
}
__VirtualTable __VirtualTable_Function = {
    .size = sizeof(__Function),
    .gc_iterator = __VirtualTable_Function_gc_iterator,
    .array.vtable = NULL,
    .array.offset = 0,
    .function.offset = 0,
    .table.size = 0
};
