#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "include.h"




static bool check_pointers(Ov_UnknownData a, Ov_UnknownData b) {
    if (a.vtable == &Ov_VirtualTable_Char) {
        if (b.vtable == a.vtable)
            return a.data.b == b.data.b;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == a.vtable)
            return a.data.f == b.data.f;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == a.vtable)
            return a.data.i == b.data.i;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Bool) {
        if (b.vtable == a.vtable)
            return a.data.b == b.data.b;
        else
            return false;
    } else {
        return a.data.ptr == b.data.ptr;
    }
}

const char getter_parameters[] = "r";
Ov_Reference_Owned getter_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_GC_Reference* reference = (Ov_GC_Reference*) args[0];
    switch (reference->type) {
        case SYMBOL: {
            reference->symbol->vtable = &Ov_VirtualTable_Object;
            reference->symbol->data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object);
            break;
        }
        case PROPERTY: {
            if (reference->property.vtable == &Ov_VirtualTable_UnknownData) {
                Ov_UnknownData* data = Ov_UnknownData_get_property(reference->property.parent, reference->property.hash);
                data->vtable = &Ov_VirtualTable_Object;
                data->data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object);
            }
            break;
        }
        case ARRAY: {
            Ov_ArrayInfo array = Ov_UnknownData_get_array(reference->array.array);
            if (array.vtable == &Ov_VirtualTable_UnknownData) {
                Ov_UnknownData* data = Ov_Array_get(array, reference->array.i);
                data->vtable = &Ov_VirtualTable_Object;
                data->data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object);
            }
            break;
        }
        default:
            break;
    }

    return Ov_Reference_copy(args[0]);
}

const char defined_parameters[] = "r";
Ov_Reference_Owned defined_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    bool defined = true;

    Ov_GC_Reference* reference = (Ov_GC_Reference*) args[0];
    switch (reference->type) {
        case SYMBOL: {
            defined = reference->symbol->vtable != NULL;
            break;
        }
        case PROPERTY: {
            if (reference->property.vtable == &Ov_VirtualTable_UnknownData) {
                Ov_UnknownData* data = Ov_UnknownData_get_property(reference->property.parent, reference->property.hash);
                defined = data->vtable != NULL;
            }
            break;
        }
        case ARRAY: {
            Ov_ArrayInfo array = Ov_UnknownData_get_array(reference->array.array);
            if (array.vtable == &Ov_VirtualTable_UnknownData) {
                Ov_UnknownData* data = Ov_Array_get(array, reference->array.i);
                defined = data->vtable != NULL;
            }
            break;
        }
        default:
            break;
    }

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = defined
    };
    return Ov_Reference_new_data(data);
}

const char setter_parameters[] = "[rr]";
static Ov_Reference_Owned assignation_body(Ov_GC_Reference* var, Ov_UnknownData data) {
    switch (var->type) {
        case DATA: {
            return Ov_Reference_new_data(data);
        }
        case SYMBOL: {
            *var->symbol = data;
            break;
        }
        case PROPERTY: {
            Ov_UnknownData_set(var->property.vtable, Ov_UnknownData_get_property(var->property.parent, var->property.hash), data);
            break;
        }
        case ARRAY: {
            Ov_ArrayInfo array = Ov_UnknownData_get_array(var->array.array);
            Ov_UnknownData_set(array.vtable, Ov_Array_get(array, var->array.i), data);
            break;
        }
        case TUPLE: {
            Ov_ArrayInfo array = Ov_UnknownData_get_array(data);
            size_t i;
            for (i = 0; i < var->tuple.size; ++i)
                assignation_body(&var->tuple.references[i], Ov_UnknownData_from_ptr(array.vtable, Ov_Array_get(array, i)));
            break;
        }
        default:
            break;
    }

    return Ov_Reference_copy((Ov_Reference_Shared) var);
}
Ov_Reference_Owned setter_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    return assignation_body((Ov_GC_Reference*) args[0], Ov_Reference_get(args[1]));
}
static bool assignation_filter(Ov_GC_Reference* var, Ov_UnknownData data) {
    if (var->type == TUPLE) {
        Ov_ArrayInfo array = Ov_UnknownData_get_array(data);
        if (var->tuple.size == array.array->size) {
            size_t i;
            for (i = 0; i < var->tuple.size; ++i)
                if (!assignation_filter(&var->tuple.references[i], Ov_UnknownData_from_ptr(array.vtable, Ov_Array_get(array, i))))
                    return false;

            return true;
        } else return false;
    } else return true;
}
bool setter_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    return assignation_filter((Ov_GC_Reference*) args[0], Ov_Reference_get(args[1]));
}

const char separator_parameters[] = "r";
Ov_Reference_Owned separator_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    return Ov_Reference_get_element(args[0], Ov_Reference_get_size(args[0]) - 1);
}

const char if_statement_parameters[] = "e";
Ov_Reference_Owned if_statement_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (local_variables);
    Ov_Reference_Shared if_word = captures[0];
    Ov_Reference_Shared else_word = captures[1];
    Ov_Expression* expression = (Ov_Expression*) args[0];

    assert(expression->type == Ov_EXPRESSION_TUPLE);
    assert(expression->tuple.size >= 2);

    Ov_Reference_Owned result = Ov_Function_execute(expression->tuple.tab[0]);
    Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(result));
    Ov_Reference_free(result);

    assert(data.vtable == &Ov_VirtualTable_Bool);
    if (data.data.b)
        return Ov_Function_execute(expression->tuple.tab[1]);
    else {
        size_t i = 2;
        while (i < expression->tuple.size) {
            result = Ov_Function_execute(expression->tuple.tab[i]);
            data = Ov_Reference_get(Ov_Reference_share(result));
            Ov_Reference_free(result);

            assert(check_pointers(data, Ov_Reference_get(Ov_Reference_share(else_word))));
            result = Ov_Function_execute(expression->tuple.tab[i + 1]);
            data = Ov_Reference_get(Ov_Reference_share(result));

            if (check_pointers(data, Ov_Reference_get(Ov_Reference_share(if_word))) && i + 3 < expression->tuple.size) {
                Ov_Reference_free(result);

                result = Ov_Function_execute(expression->tuple.tab[i + 2]);
                data = Ov_Reference_get(Ov_Reference_share(result));
                Ov_Reference_free(result);

                assert(data.vtable == &Ov_VirtualTable_Bool);
                if (data.data.b)
                    return Ov_Function_execute(expression->tuple.tab[i + 3]);
                else i += 4;
            } else return result;
        }
        return Ov_Reference_new_uninitialized();
    }
}

const char while_statement_parameters[] = "[r()r()]";
Ov_Reference_Owned while_statement_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_Reference_Shared condition = args[0];
    Ov_Reference_Shared body = args[1];

    Ov_Reference_Owned result = Ov_Reference_new_uninitialized();

    Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
    Ov_Expression ex = {
        .type = Ov_EXPRESSION_REFERENCE,
        .reference = Ov_Reference_share(param)
    };
    while (true) {
        Ov_Reference_Owned c = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(condition)), ex);
        Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(c));

        assert(data.vtable == &Ov_VirtualTable_Bool);
        if (data.data.b) {
            Ov_Reference_Owned r = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(body)), ex);
            Ov_Reference_free(result);
            result = r;
        } else break;

        Ov_Reference_free(c);
    }

    Ov_Reference_free(param);
    return result;
}

const char copy_parameters[] = "r";
bool copy_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_VirtualTable* vtable = Ov_Reference_get(args[0]).vtable;

    return
        vtable == &Ov_VirtualTable_Int ||
        vtable == &Ov_VirtualTable_Float ||
        vtable == &Ov_VirtualTable_Char ||
        vtable == &Ov_VirtualTable_Bool;
}
Ov_Reference_Owned copy_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData data = Ov_Reference_get(args[0]);

    return Ov_Reference_new_data(data);
}

const char copy_pointer_parameters[] = "r";
Ov_Reference_Owned copy_pointer_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData data = Ov_Reference_get(args[0]);

    return Ov_Reference_new_data(data);
}

const char function_definition_parameters[] = "[rr]";
Ov_Reference_Owned function_definition_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_Function function = *Ov_UnknownData_get_function(Ov_Reference_get(args[1]));

    Ov_Reference_Owned references[function->captures.size];
    size_t i;
    for (i = 0; i < function->captures.size; ++i)
        references[i] = Ov_GC_capture_to_reference(function->captures.tab[i]);
    Ov_Function_push(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), function->parameters, function->body, function->filter, function->local_variables, (Ov_Reference_Shared*) references, function->captures.size);

    for (i = 0; i < function->captures.size; ++i)
        Ov_Reference_free(references[i]);
    return Ov_Reference_copy(args[0]);
}

static bool equals(Ov_UnknownData a, Ov_UnknownData b) {
    if (a.vtable == &Ov_VirtualTable_Char) {
        if (b.vtable == a.vtable)
            return a.data.b == b.data.b;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == a.vtable)
            return a.data.f == b.data.f;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == a.vtable)
            return a.data.i == b.data.i;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Bool) {
        if (b.vtable == a.vtable)
            return a.data.b == b.data.b;
        else
            return false;
    } else {
        // TODO
    }
}

const char equals_parameters[] = "[rr]";
Ov_Reference_Owned equals_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = equals(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char not_equals_parameters[] = "[rr]";
Ov_Reference_Owned not_equals_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = !equals(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char check_pointers_parameters[] = "[rr]";
Ov_Reference_Owned check_pointers_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = check_pointers(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char not_check_pointers_parameters[] = "[rr]";
Ov_Reference_Owned not_check_pointers_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = !check_pointers(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char string_from_parameters[] = "r";
extern Ov_VirtualTable* Ov_VirtualTable_string_from_tuple;
Ov_Reference_Owned string_from_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData string = {
        .vtable = Ov_VirtualTable_string_from_tuple,
        .data.ptr = Ov_GC_alloc_object(Ov_VirtualTable_string_from_tuple)
    };
    Ov_ArrayInfo string_array = Ov_UnknownData_get_array(string);

    Ov_UnknownData data = Ov_Reference_get(args[0]);
    if (data.vtable == &Ov_VirtualTable_Int ||
        data.vtable == &Ov_VirtualTable_Float ||
        data.vtable == &Ov_VirtualTable_Char ||
        data.vtable == &Ov_VirtualTable_Bool) {

        char buffer[32];
        if (data.vtable == &Ov_VirtualTable_Int)
            sprintf(buffer, "%li", data.data.i);
        else if (data.vtable == &Ov_VirtualTable_Float)
            sprintf(buffer, "%F", data.data.f);
        else if (data.vtable == &Ov_VirtualTable_Char)
            sprintf(buffer, "%c", data.data.c);
        else if (data.vtable == &Ov_VirtualTable_Bool)
            sprintf(buffer, data.data.c ? "true" : "false");

        Ov_Array_set_size(string_array, strlen(buffer));
        size_t i;
        for (i = 0; i < string_array.array->size; ++i) {
            Ov_UnknownData c = {
                .vtable = &Ov_VirtualTable_Char,
                .data.c = buffer[i]
            };
            Ov_UnknownData_set(string_array.vtable, Ov_Array_get(string_array, i), c);
        }
    } else {
        Ov_ArrayInfo array = Ov_UnknownData_get_array(data);
        size_t i;
        for (i = 0; i < array.array->size; ++i) {
            Ov_Reference_Owned r = Ov_Reference_new_array(data, i);
            Ov_Expression ex = {
                .type = Ov_EXPRESSION_REFERENCE,
                .reference = Ov_Reference_share(r)
            };
            Ov_Reference_Owned str = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(Ov_Reference_share(string_from))), ex);
            Ov_ArrayInfo str_array = Ov_UnknownData_get_array(Ov_Reference_get(Ov_Reference_share(str)));

            size_t begin = string_array.array->size;
            Ov_Array_set_size(string_array, string_array.array->size + str_array.array->size);
            size_t j;
            for (j = 0; j < str_array.array->size; ++j)
                Ov_UnknownData_set(
                    string_array.vtable,
                    Ov_Array_get(string_array, begin + j),
                    Ov_UnknownData_from_ptr(str_array.vtable, Ov_Array_get(str_array, j))
                );

            Ov_Reference_free(str);
            Ov_Reference_free(r);
        }
    }

    return Ov_Reference_new_data(string);
}

const char print_parameters[] = "r";
Ov_Reference_Owned print_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_Reference_Shared ref = args[0];

    Ov_Expression ex = {
        .type = Ov_EXPRESSION_REFERENCE,
        .reference = ref
    };
    Ov_Reference_Owned str = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(Ov_Reference_share(string_from))), ex);
    Ov_ArrayInfo array = Ov_UnknownData_get_array(Ov_Reference_get(Ov_Reference_share(str)));

    size_t i;
    for (i = 0; i < array.array->size; ++i)
        printf("%c", Ov_UnknownData_from_ptr(array.vtable, Ov_Array_get(array, i)).data.c);
    printf("\n");

    Ov_Reference_free(str);
    return Ov_Reference_new_uninitialized();
}

const char logical_not_parameters[] = "r";
bool logical_not_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData data = Ov_Reference_get(args[0]);

    return data.vtable == &Ov_VirtualTable_Bool;
}
Ov_Reference_Owned logical_not_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData data = Ov_Reference_get(args[0]);

    data.data.b = !data.data.b;
    return Ov_Reference_new_data(data);
}

const char logical_and_parameters[] = "[rr()]";
Ov_Reference_Owned logical_and_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b_lambda = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = &Ov_VirtualTable_Bool;

    if (a.data.b) {
        Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
        Ov_Expression ex = {
            .type = Ov_EXPRESSION_REFERENCE,
            .reference = Ov_Reference_share(param)
        };
        Ov_Reference_Owned c = Ov_Function_eval(Ov_UnknownData_get_function(b_lambda), ex);

        r.data.b = Ov_Reference_get(Ov_Reference_share(c)).data.b;

        Ov_Reference_free(c);
        Ov_Reference_free(param);
    } else r.data.b = false;

    return Ov_Reference_new_data(r);
}

const char logical_or_parameters[] = "[rr()]";
Ov_Reference_Owned logical_or_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b_lambda = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = &Ov_VirtualTable_Bool;

    if (!a.data.b) {
        Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
        Ov_Expression ex = {
            .type = Ov_EXPRESSION_REFERENCE,
            .reference = Ov_Reference_share(param)
        };
        Ov_Reference_Owned c = Ov_Function_eval(Ov_UnknownData_get_function(b_lambda), ex);

        r.data.b = Ov_Reference_get(Ov_Reference_share(c)).data.b;

        Ov_Reference_free(c);
        Ov_Reference_free(param);
    } else r.data.b = true;

    return Ov_Reference_new_data(r);
}

const char addition_parameters[] = "[rr]";
Ov_Reference_Owned addition_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = NULL;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Int;
            r.data.i = a.data.i + b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.i + b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f + b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f + b.data.f;
        }
    }
    assert(r.vtable != NULL);

    return Ov_Reference_new_data(r);
}

const char opposite_parameters[] = "r";
Ov_Reference_Owned opposite_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    Ov_UnknownData r;
    r.vtable = NULL;
    if (a.vtable == &Ov_VirtualTable_Int) {
        r.vtable = &Ov_VirtualTable_Int;
        r.data.i = -a.data.i;
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        r.vtable = &Ov_VirtualTable_Float;
        r.data.i = -a.data.f;
    }
    assert(r.vtable != NULL);

    return Ov_Reference_new_data(r);
}

const char substraction_parameters[] = "[rr]";
Ov_Reference_Owned substraction_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = NULL;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Int;
            r.data.i = a.data.i - b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.i - b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f - b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f - b.data.f;
        }
    }
    assert(r.vtable != NULL);

    return Ov_Reference_new_data(r);
}

const char multiplication_parameters[] = "[rr]";
Ov_Reference_Owned multiplication_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = NULL;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Int;
            r.data.i = a.data.i * b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.i * b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f * b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f * b.data.f;
        }
    }
    assert(r.vtable != NULL);

    return Ov_Reference_new_data(r);
}

const char division_parameters[] = "[rr]";
Ov_Reference_Owned division_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = NULL;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Int;
            r.data.i = a.data.i / b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.i / b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f / b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            r.vtable = &Ov_VirtualTable_Float;
            r.data.f = a.data.f / b.data.f;
        }
    }
    assert(r.vtable != NULL);

    return Ov_Reference_new_data(r);
}

const char modulo_parameters[] = "[rr]";
Ov_Reference_Owned modulo_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    Ov_UnknownData r;
    r.vtable = NULL;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            r.vtable = &Ov_VirtualTable_Int;
            r.data.i = a.data.i % b.data.i;
        }
    }
    assert(r.vtable != NULL);

    return Ov_Reference_new_data(r);
}


Ov_Reference_Owned getter;
Ov_Reference_Owned defined;
Ov_Reference_Owned setter;
Ov_Reference_Owned _x3A_x3D;
Ov_Reference_Owned _x3B;
Ov_Reference_Owned if_statement;
Ov_Reference_Owned else_statement;
Ov_Reference_Owned while_statement;
Ov_Reference_Owned _x24;
Ov_Reference_Owned _x24_x3D_x3D;
Ov_Reference_Owned _x3A;
Ov_Reference_Owned _x3D_x3D;
Ov_Reference_Owned _x21_x3D;
Ov_Reference_Owned _x3D_x3D_x3D;
Ov_Reference_Owned _x21_x3D_x3D;
Ov_Reference_Owned string_from;
Ov_Reference_Owned print;
Ov_Reference_Owned _x21;
Ov_Reference_Owned _x26;
Ov_Reference_Owned _x7C;
Ov_Reference_Owned _x2B;
Ov_Reference_Owned _x2D;
Ov_Reference_Owned _x2A;
Ov_Reference_Owned _x2F;
Ov_Reference_Owned _x25;

void Ov_init_functions() {
    Ov_UnknownData getter_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(getter_data), getter_parameters, getter_body, NULL, 0, NULL, 0);
    getter = Ov_Reference_new_symbol(getter_data);

    Ov_UnknownData defined_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(defined_data), defined_parameters, defined_body, NULL, 0, NULL, 0);
    defined = Ov_Reference_new_symbol(defined_data);

    Ov_UnknownData setter_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(setter_data), setter_parameters, setter_body, NULL, 0, NULL, 0);
    setter = Ov_Reference_new_symbol(setter_data);
    _x3A_x3D = Ov_Reference_new_symbol(setter_data);

    Ov_UnknownData separator_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(separator_data), separator_parameters, separator_body, NULL, 0, NULL, 0);
    _x3B = Ov_Reference_new_symbol(separator_data);

    Ov_UnknownData else_statement_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    else_statement = Ov_Reference_new_symbol(else_statement_data);

    Ov_UnknownData if_statement_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    if_statement = Ov_Reference_new_symbol(if_statement_data);
    Ov_Reference_Shared if_statement_captures[] = {
        Ov_Reference_share(if_statement),
        Ov_Reference_share(else_statement)
    };
    Ov_Function_push(Ov_UnknownData_get_function(if_statement_data), if_statement_parameters, if_statement_body, NULL, 0, if_statement_captures, sizeof(if_statement_captures) / sizeof(if_statement_captures[0]));

    Ov_UnknownData while_statement_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(while_statement_data), while_statement_parameters, while_statement_body, NULL, 0, NULL, 0);
    while_statement = Ov_Reference_new_symbol(while_statement_data);

    Ov_UnknownData copy_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(copy_data), copy_parameters, copy_body, NULL, 0, NULL, 0);
    _x24 = Ov_Reference_new_symbol(copy_data);

    Ov_UnknownData copy_pointer_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(copy_pointer_data), copy_pointer_parameters, copy_pointer_body, NULL, 0, NULL, 0);
    _x24_x3D_x3D = Ov_Reference_new_symbol(copy_pointer_data);

    Ov_UnknownData function_definition_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(function_definition_data), function_definition_parameters, function_definition_body, NULL, 0, NULL, 0);
    _x24_x3D_x3D = Ov_Reference_new_symbol(function_definition_data);

    Ov_UnknownData equals_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(equals_data), equals_parameters, equals_body, NULL, 0, NULL, 0);
    _x3D_x3D = Ov_Reference_new_symbol(equals_data);

    Ov_UnknownData check_pointers_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(check_pointers_data), check_pointers_parameters, check_pointers_body, NULL, 0, NULL, 0);
    _x3D_x3D_x3D = Ov_Reference_new_symbol(check_pointers_data);

    Ov_UnknownData not_check_pointers_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(not_check_pointers_data), not_check_pointers_parameters, not_check_pointers_body, NULL, 0, NULL, 0);
    _x21_x3D_x3D = Ov_Reference_new_symbol(not_check_pointers_data);

    Ov_UnknownData string_from_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(string_from_data), string_from_parameters, string_from_body, NULL, 0, NULL, 0);
    string_from = Ov_Reference_new_symbol(string_from_data);

    Ov_UnknownData print_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(print_data), print_parameters, print_body, NULL, 0, NULL, 0);
    print = Ov_Reference_new_symbol(print_data);

    Ov_UnknownData logical_not_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(logical_not_data), logical_not_parameters, logical_not_body, NULL, 0, NULL, 0);
    _x21 = Ov_Reference_new_symbol(logical_not_data);

    Ov_UnknownData logical_and_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(logical_and_data), logical_and_parameters, logical_and_body, NULL, 0, NULL, 0);
    _x26 = Ov_Reference_new_symbol(logical_and_data);

    Ov_UnknownData logical_or_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(logical_or_data), logical_or_parameters, logical_or_body, NULL, 0, NULL, 0);
    _x7C = Ov_Reference_new_symbol(logical_or_data);

    Ov_UnknownData addition_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(addition_data), addition_parameters, addition_body, NULL, 0, NULL, 0);
    _x2B = Ov_Reference_new_symbol(addition_data);

    Ov_UnknownData minus_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(minus_data), opposite_parameters, opposite_body, NULL, 0, NULL, 0);
    Ov_Function_push(Ov_UnknownData_get_function(minus_data), substraction_parameters, substraction_body, NULL, 0, NULL, 0);
    _x2D = Ov_Reference_new_symbol(minus_data);

    Ov_UnknownData multiplication_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(multiplication_data), multiplication_parameters, multiplication_body, NULL, 0, NULL, 0);
    _x2A = Ov_Reference_new_symbol(multiplication_data);

    Ov_UnknownData division_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(division_data), division_parameters, division_body, NULL, 0, NULL, 0);
    _x2F = Ov_Reference_new_symbol(division_data);

    Ov_UnknownData modulo_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(modulo_data), modulo_parameters, modulo_body, NULL, 0, NULL, 0);
    _x25 = Ov_Reference_new_symbol(modulo_data);
}
