#include <stdlib.h>

#include "hash_string.h"
#include "include.h"
#include "gc.h"


struct __FunctionCapture __GC_reference_to_capture(__GC_Reference* reference) {
    struct __FunctionCapture capture;

    switch (reference->type) {
    case DATA: {
        __UnknownData* d = __GC_alloc_object(&__VirtualTable_UnknownData);
        *d = reference->data;

        capture.type = __FUNCTIONCAPTURE_SYMBOL;
        capture.symbol = d;
        break;
    }
    case SYMBOL: {
        capture.type = __FUNCTIONCAPTURE_SYMBOL;
        capture.symbol = reference->symbol;
        break;
    }
    case PROPERTY: {
        capture.type = __FUNCTIONCAPTURE_PROPERTY;
        capture.property.parent = reference->property.parent;
        capture.property.virtual_table = reference->property.virtual_table;
        capture.property.hash = reference->property.hash;
        break;
    }
    case ARRAY: {
        capture.type = __FUNCTIONCAPTURE_ARRAY;
        capture.array.array = reference->array.array;
        capture.array.i = reference->array.i;
        break;
    }
    case TUPLE: {
        __UnknownData* d = __GC_alloc_object(&__VirtualTable_UnknownData);
        *d = __Reference_get((__Reference_Shared)reference);

        capture.type = __FUNCTIONCAPTURE_SYMBOL;
        capture.symbol = d;
        break;
    }
    default:
        break;
    }

    return capture;
}

__Reference_Owned __GC_capture_to_reference(struct __FunctionCapture capture) {
    switch (capture.type) {
    case __FUNCTIONCAPTURE_SYMBOL: {
        __GC_Reference* reference = __GC_alloc_references(1);
        reference->type = SYMBOL;
        reference->symbol = capture.symbol;

        return (__Reference_Owned)reference;
    }
    case __FUNCTIONCAPTURE_PROPERTY:
        return __Reference_new_property(capture.property.parent, capture.property.virtual_table, capture.property.hash);
    case __FUNCTIONCAPTURE_ARRAY:
        return __Reference_new_array(capture.array.array, capture.array.i);
    default:
        return NULL;
    }
}

__Function __Function_new() {
    return NULL;
}

__Function __Function_copy(__Function function) {
    __Function cpy = NULL;

    __FunctionCell* cell;
    for (cell = function; cell != NULL; cell = cell->next) {
        __FunctionCell* f = malloc(sizeof(__Function) + cell->captures.size * sizeof(__Reference_Owned));

        f->next = cpy;
        f->parameters = cell->parameters;
        f->filter = cell->filter;
        f->body = cell->body;
        f->captures.size = cell->captures.size;

        size_t i;
        for (i = 0; i < cell->captures.size; ++i)
            f->captures.tab[i] = cell->captures.tab[i];

        cpy = f;
    }

    return cpy;
}

void __Function_push(__Function* function, const char* parameters, __FunctionBody body, __FunctionFilter filter, __Reference_Owned captures[], size_t captures_size) {
    __FunctionCell* f = malloc(sizeof(__Function) + captures_size * sizeof(__Reference_Owned));

    f->next = *function;
    f->parameters = parameters;
    f->filter = filter;
    f->body = body;
    f->captures.size = captures_size;

    size_t i;
    for (i = 0; i < captures_size; ++i)
        f->captures.tab[i] = __GC_reference_to_capture((__GC_Reference*)captures[i]);

    *function = f;
}

void __Function_pop(__Function* function) {
    __FunctionCell* f = *function;
    __FunctionCell* next = f->next;

    free(*function);

    *function = next;
}

void __Function_free(__Function* function) {
    while (*function != NULL)
        __Function_pop(function);
}

__Reference_Owned __Function_execute(__Expression args) {
    switch (args.type) {
    case __EXPRESSION_TUPLE: {
        __Reference_Owned ref[args.tuple.size];

        size_t i;
        for (i = 0; i < args.tuple.size; ++i)
            ref[i] = __Function_execute(args.tuple.tab[i]);

        __Reference_Owned r = __Reference_new_tuple((__Reference_Shared*)ref, args.tuple.size);

        for (i = 0; i < args.tuple.size; ++i)
            __Reference_free(ref[i]);

        return r;
    }
    case __EXPRESSION_REFERENCE: {
        return __Reference_copy(args.reference);
    }
    case __EXPRESSION_LAMBDA: {
        __Expression expr = {
            .type = __EXPRESSION_TUPLE,
            .tuple.size = 0
        };
        return __Function_eval(&args.lambda, expr);
    }
    default:
        return NULL;
    }
}

bool __Function_parse(__Reference_Owned captures[], __Reference_Shared* vars, bool* owned, size_t* i, const char** params, __Expression args) {
    if (**params == 'r') {
        ++(*params);

        if (**params == '(') {
            ++(*params);

            union __Data data = {
                .ptr = __GC_alloc_object(&__VirtualTable_Function)
            };
            vars[*i] = (__Reference_Shared)__Reference_new_data(__UnknownData_from_data(&__VirtualTable_Function, data));
            owned[*i] = true;

            *((__Function*)data.ptr) = __Function_copy(args.lambda);
        }
        else if (**params == '.') {
            ++(*params);

            unsigned int n = 0;
            do {
                n *= 10;
                n += **params - '0';
                ++(*params);
            } while ('0' < **params && **params > '9');

            __GC_Reference* r = (__GC_Reference*)__Function_execute(args);

            if (r->type == PROPERTY && (r->property.hash == n || n == hash("."))) {
                vars[*i] = (__Reference_Shared)__Reference_new_data(r->property.parent);
                owned[*i] = true;
                __Reference_free((__Reference_Owned)r);
            }
            else {
                __Reference_free((__Reference_Owned)r);
                return false;
            }
        }
        else {
            if (args.type == __EXPRESSION_REFERENCE) {
                vars[*i] = args.reference;
                owned[*i] = false;
            }
            else {
                vars[*i] = (__Reference_Shared)__Function_execute(args);
                owned[*i] = true;
            }
        }

        ++(*i);
        return true;
    }
    else if (**params == '(') {
        size_t j = 0;
        size_t size;

        switch (args.type) {
        case __EXPRESSION_TUPLE: {
            size = args.tuple.size;
            while (**params != ')' && j < args.tuple.size) {
                ++(*params);
                __Function_parse(captures, vars, owned, i, params, args.tuple.tab[j]);
                ++j;
            }
            break;
        }
        case __EXPRESSION_REFERENCE: {
            size = __Reference_get_size(args.reference);
            while (**params != ')' && j < size) {
                ++(*params);
                __Expression expr = {
                    .type = __EXPRESSION_REFERENCE,
                    .reference = __Reference_share(__Reference_get_element(args.reference, j))
                };
                __Function_parse(captures, vars, owned, i, params, expr);
                ++j;
            }
            break;
        }
        case __EXPRESSION_LAMBDA: {
            __Reference_Owned ref = __Function_execute(args);
            size = __Reference_get_size(__Reference_share(ref));
            while (**params != ')' && j < size) {
                ++(*params);
                __Expression expr = {
                    .type = __EXPRESSION_REFERENCE,
                    .reference = __Reference_share(__Reference_get_element(__Reference_share(ref), j))
                };
                __Function_parse(captures, vars, owned, i, params, expr);
                ++j;
            }
            __Reference_free(ref);
            break;
        }
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

        __Function* f = __UnknownData_get_function(__Reference_get(__Reference_share(captures[n])));
        __Function_eval(f, args);
    }

    return false;
}

__Reference_Owned __Function_eval(__Function* function, __Expression args) {
    __FunctionCell* ptr;
    for (ptr = *function; ptr != NULL; ptr = ptr->next) {
        const char* c;

        size_t size = 0;
        for (c = ptr->parameters; *c != '\0'; c++)
            if (*c == 'r')
                ++size;
        __Reference_Shared vars[size];
        bool owned[size];

        __Reference_Owned captures[ptr->captures.size];
        size_t j;
        for (j = 0; j < ptr->captures.size; ++j)
            captures[j] = __GC_capture_to_reference(ptr->captures.tab[j]);

        size_t i = 0;
        c = ptr->parameters;
        bool parsed = __Function_parse(captures, vars, owned, &i, &c, args);

        __Reference_Owned ref = NULL;
        if (parsed && (ptr->filter == NULL || ptr->filter(captures, vars)))
            ref = ptr->body(captures, vars);

        for (j = 0; j < ptr->captures.size; ++j)
            __Reference_free(captures[j]);

        for (i = 0; i < size; ++i)
            if (owned[i])
                __Reference_free((__Reference_Owned)vars[i]);

        if (ref != NULL)
            return ref;
    }

    return NULL;
    //throw exception
}

void __VirtualTable_Function_gc_iterator(void* ptr) {
    __FunctionCell* f;
    for (f = *((__Function*)ptr); f != NULL; f = f->next) {
        size_t i;
        for (i = 0; i < f->captures.size; ++i) {
            __Reference_Owned ref = __GC_capture_to_reference(f->captures.tab[i]);
            __UnknownData data = __Reference_get(__Reference_share(ref));
            __GC_iterate(__VirtualTable_UnknownData.gc_iterator, &data);
            __Reference_free(ref);
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
