#include <assert.h>
#include <stdlib.h>

#include <ouverium/hash_string.h>
#include <ouverium/include.h>

#include "gc.h"


struct Ov_FunctionCapture Ov_GC_reference_to_capture(Ov_GC_Reference* reference) {
    struct Ov_FunctionCapture capture;

    switch (reference->type) {
        case DATA: {
            Ov_UnknownData* d = Ov_GC_alloc_object(&Ov_VirtualTable_UnknownData);
            *d = reference->data;

            capture.type = Ov_FUNCTIONCAPTURE_SYMBOL;
            capture.symbol = d;
            break;
        }
        case SYMBOL: {
            capture.type = Ov_FUNCTIONCAPTURE_SYMBOL;
            capture.symbol = reference->symbol;
            break;
        }
        case PROPERTY: {
            capture.type = Ov_FUNCTIONCAPTURE_PROPERTY;
            capture.property.parent = reference->property.parent;
            capture.property.hash = reference->property.hash;
            break;
        }
        case ARRAY: {
            capture.type = Ov_FUNCTIONCAPTURE_ARRAY;
            capture.array.array = reference->array.array;
            capture.array.i = reference->array.i;
            break;
        }
        case TUPLE: {
            Ov_UnknownData* d = Ov_GC_alloc_object(&Ov_VirtualTable_UnknownData);
            *d = Ov_Reference_get((Ov_Reference_Shared) reference);

            capture.type = Ov_FUNCTIONCAPTURE_SYMBOL;
            capture.symbol = d;
            break;
        }
        default:
            break;
    }

    return capture;
}

Ov_Reference_Owned Ov_GC_capture_to_reference(struct Ov_FunctionCapture capture) {
    switch (capture.type) {
        case Ov_FUNCTIONCAPTURE_SYMBOL: {
            Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
            reference->type = SYMBOL;
            reference->symbol = capture.symbol;

            return (Ov_Reference_Owned) reference;
        }
        case Ov_FUNCTIONCAPTURE_PROPERTY:
            return Ov_Reference_new_property(capture.property.parent, capture.property.hash);
        case Ov_FUNCTIONCAPTURE_ARRAY:
            return Ov_Reference_new_array(capture.array.array, capture.array.i);
        default:
            return NULL;
    }
}

Ov_Function Ov_Function_new() {
    return NULL;
}

Ov_Function Ov_Function_copy(Ov_Function function) {
    Ov_Function cpy = NULL;

    Ov_FunctionCell* cell;
    for (cell = function; cell != NULL; cell = cell->next) {
        Ov_FunctionCell* f = malloc(sizeof(Ov_FunctionCell) + cell->captures.size * sizeof(struct Ov_FunctionCapture));

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

void Ov_Function_push(Ov_Function* function, const char* parameters, Ov_FunctionBody body, Ov_FunctionFilter filter, size_t local_variables, Ov_Reference_Shared captures[], size_t captures_size) {
    Ov_FunctionCell* f = malloc(sizeof(Ov_FunctionCell) + captures_size * sizeof(struct Ov_FunctionCapture));

    f->next = *function;
    f->parameters = parameters;
    f->filter = filter;
    f->body = body;
    f->local_variables = local_variables;
    f->captures.size = captures_size;

    size_t i;
    for (i = 0; i < captures_size; ++i)
        f->captures.tab[i] = Ov_GC_reference_to_capture((Ov_GC_Reference*) captures[i]);

    *function = f;
}

void Ov_Function_pop(Ov_Function* function) {
    Ov_FunctionCell* f = *function;
    Ov_FunctionCell* next = f->next;

    free(*function);

    *function = next;
}

void Ov_Function_free(Ov_Function* function) {
    while (*function != NULL)
        Ov_Function_pop(function);
}

Ov_Reference_Owned Ov_Function_execute(Ov_Expression args) {
    switch (args.type) {
        case Ov_EXPRESSION_TUPLE: {
            Ov_Reference_Owned ref[args.tuple.size];

            size_t i;
            for (i = 0; i < args.tuple.size; ++i)
                ref[i] = Ov_Function_execute(args.tuple.tab[i]);

            Ov_Reference_Owned r = Ov_Reference_new_tuple((Ov_Reference_Shared*) ref, args.tuple.size, args.tuple.vtable);

            for (i = 0; i < args.tuple.size; ++i)
                Ov_Reference_free(ref[i]);

            return r;
        }
        case Ov_EXPRESSION_REFERENCE: {
            return Ov_Reference_copy(args.reference);
        }
        case Ov_EXPRESSION_LAMBDA: {
            Ov_Reference_Owned ref = Ov_Reference_new_uninitialized();
            Ov_Expression expr = {
                .type = Ov_EXPRESSION_REFERENCE,
                .reference = Ov_Reference_share(ref)
            };
            Ov_Reference_Owned result = Ov_Function_eval(&args.lambda, expr);
            Ov_Reference_free(ref);
            Ov_Function_free(&args.lambda);
            return result;
        }
        default:
            return NULL;
    }
}

static Ov_Reference_Owned simple_lambda(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (args);
    (void) (local_variables);
    return Ov_Reference_copy(Ov_Reference_share(captures[0]));
}

bool Ov_Function_parse(Ov_Reference_Shared captures[], Ov_Reference_Shared* vars, bool* owned, size_t* i, const char** params, Ov_Expression* args) {
    if (**params == 'r') {
        ++(*params);

        if (**params == '(') {
            (*params) += 2;

            Ov_UnknownData data = {
                .vtable = &Ov_VirtualTable_Function,
                .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
            };
            if (args->type == Ov_EXPRESSION_LAMBDA) {
                *Ov_UnknownData_get_function(data) = args->lambda;
            } else {
                Ov_Reference_Owned r = Ov_Function_execute(*args);
                Ov_Reference_Shared captures[] = { Ov_Reference_share(r) };
                Ov_Function_push(Ov_UnknownData_get_function(data), "", simple_lambda, NULL, 0, captures, 1);
                Ov_Reference_free(r);
            }
            vars[*i] = (Ov_Reference_Shared) Ov_Reference_new_data(data);
            owned[*i] = true;
        } else if (**params == '.') {
            ++(*params);

            unsigned n = 0;
            do {
                n *= 10;
                n += **params - '0';
                ++(*params);
            } while ('0' < **params && **params > '9');

            Ov_GC_Reference* r = (Ov_GC_Reference*) Ov_Function_execute(*args);

            if (r->type == PROPERTY && (r->property.hash == n || n == hash_string("."))) {
                vars[*i] = (Ov_Reference_Shared) Ov_Reference_new_data(r->property.parent);
                owned[*i] = true;
                Ov_Reference_free((Ov_Reference_Owned) r);
            } else {
                Ov_Reference_free((Ov_Reference_Owned) r);
                return false;
            }
        } else {
            if (args->type == Ov_EXPRESSION_REFERENCE) {
                vars[*i] = args->reference;
                owned[*i] = false;
            } else {
                vars[*i] = (Ov_Reference_Shared) Ov_Function_execute(*args);
                owned[*i] = true;
            }
        }

        ++(*i);
        return true;
    } else if (**params == '[') {
        ++(*params);

        size_t j = 0;
        size_t size;

        switch (args->type) {
            case Ov_EXPRESSION_TUPLE: {
                size = args->tuple.size;
                while (**params != ']' && j < args->tuple.size) {
                    Ov_Function_parse(captures, vars, owned, i, params, &(args->tuple.tab[j]));
                    ++j;
                }
                break;
            }
            case Ov_EXPRESSION_REFERENCE: {
                size = Ov_Reference_get_size(args->reference);
                while (**params != ']' && j < size) {
                    Ov_Expression expr = {
                        .type = Ov_EXPRESSION_REFERENCE,
                        .reference = Ov_Reference_share(Ov_Reference_get_element(args->reference, j))
                    };
                    Ov_Function_parse(captures, vars, owned, i, params, &expr);
                    ++j;
                }
                break;
            }
            case Ov_EXPRESSION_LAMBDA: {
                Ov_Reference_Owned ref = Ov_Function_execute(*args);
                size = Ov_Reference_get_size(Ov_Reference_share(ref));
                while (**params != ']' && j < size) {
                    Ov_Expression expr = {
                        .type = Ov_EXPRESSION_REFERENCE,
                        .reference = Ov_Reference_share(Ov_Reference_get_element(Ov_Reference_share(ref), j))
                    };
                    Ov_Function_parse(captures, vars, owned, i, params, &expr);
                    ++j;
                }
                Ov_Reference_free(ref);
                break;
            }
        }

        if (**params == ']' && j == size) {
            ++(*params);
            return true;
        } else
            return false;
    } else if ('0' <= **params && **params <= '9') {
        size_t n = 0;
        do {
            n *= 10;
            n += **params - '0';
            ++(*params);
        } while ('0' < **params && **params > '9');

        Ov_Function* f = Ov_UnknownData_get_function(Ov_Reference_get(captures[n]));
        Ov_Function_eval(f, *args);
    } else if (**params == 'e') {
        vars[*i] = (Ov_Reference_Shared) args;
        owned[*i] = false;
    }

    return true;
}

Ov_TryEvalResult Ov_Function_try_eval(Ov_Function* function, Ov_Expression args) {
    if (*function == NULL) {
        Ov_TryEvalResult result = {
            .correct = false,
            .reference = NULL
        };
        return result;
    }

    Ov_FunctionCell* ptr;
    for (ptr = *function; ptr != NULL; ptr = ptr->next) {
        const char* c;

        size_t size = 0;
        for (c = ptr->parameters; *c != '\0'; ++c)
            if (*c == 'r' || *c == 'e')
                ++size;
        Ov_Reference_Shared vars[size];
        bool owned[size];

        Ov_Reference_Shared captures[ptr->captures.size];
        size_t j;
        for (j = 0; j < ptr->captures.size; ++j)
            captures[j] = (Ov_Reference_Shared) Ov_GC_capture_to_reference(ptr->captures.tab[j]);

        size_t i = 0;
        c = ptr->parameters;
        bool parsed = Ov_Function_parse(captures, vars, owned, &i, &c, &args);

        Ov_Reference_Shared local_variables[ptr->local_variables];
        for (j = 0; j < ptr->local_variables; ++j)
            local_variables[j] = (Ov_Reference_Shared) Ov_Reference_new_uninitialized();

        Ov_Reference_Owned ref = NULL;
        if (parsed && (ptr->filter == NULL || ptr->filter(captures, vars, local_variables)))
            ref = ptr->body(captures, vars, local_variables);

        for (j = 0; j < ptr->local_variables; ++j)
            Ov_Reference_free((Ov_Reference_Owned) local_variables[j]);

        for (j = 0; j < ptr->captures.size; ++j)
            Ov_Reference_free((Ov_Reference_Owned) captures[j]);

        for (i = 0; i < size; ++i)
            if (owned[i])
                Ov_Reference_free((Ov_Reference_Owned) vars[i]);

        if (ref != NULL) {
            Ov_TryEvalResult result = {
                .correct = true,
                .reference = ref
            };
            return result;
        }
    }

    Ov_TryEvalResult result = {
        .correct = false,
        .reference = NULL
    };
    return result;
}

Ov_Reference_Owned Ov_Function_eval(Ov_Function* function, Ov_Expression args) {
    Ov_TryEvalResult result = Ov_Function_try_eval(function, args);
    if (result.correct)
        return result.reference;
    else {
        assert(false); //throw exception
        return NULL;
    }
}

void Ov_VirtualTable_Function_gc_iterator(void* ptr) {
    Ov_FunctionCell* f;
    for (f = *((Ov_Function*) ptr); f != NULL; f = f->next) {
        size_t i;
        for (i = 0; i < f->captures.size; ++i) {
            Ov_Reference_Owned ref = Ov_GC_capture_to_reference(f->captures.tab[i]);
            Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(ref));
            Ov_GC_iterate(Ov_VirtualTable_UnknownData.gc_iterator, &data);
            Ov_Reference_free(ref);
        }
    }
}
Ov_VirtualTable Ov_VirtualTable_Function = {
    .size = sizeof(Ov_Function),
    .gc_iterator = Ov_VirtualTable_Function_gc_iterator,
    .array.vtable = NULL,
    .array.offset = -1,
    .function.offset = 0,
    .table_size = 0
};
