#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ouverium/include.h>

#include "../gc.h"


const char getter_parameters[] = "r";
bool getter_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);

    Ov_UnknownData data = Ov_Reference_raw(args[0]);
    if (data.vtable != NULL) {
        ((Ov_GC_Reference*) local_variables[0])->data = data;
        return true;
    } else
        return false;
}
Ov_Reference_Owned getter_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (args);

    return Ov_Reference_new_data(local_variables[0]->data);
}

const char function_getter_parameters[] = "r";
Ov_Reference_Owned function_getter_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    return Ov_Reference_copy(args[0]);
}

const char defined_parameters[] = "r";
Ov_Reference_Owned defined_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = Ov_Reference_raw(args[0]).vtable != NULL
    };
    return Ov_Reference_new_data(data);
}

const char setter_parameters[] = "[rr]";
Ov_Reference_Owned assignation_body(Ov_GC_Reference* var, Ov_UnknownData data) {
    switch (var->type) {
        case DATA: {
            return Ov_Reference_new_data(data);
        }
        case SYMBOL: {
            *var->symbol = data;
            break;
        }
        case PROPERTY: {
            Ov_PropertyInfo property = Ov_UnknownData_get_property(var->property.parent, var->property.hash);
            Ov_UnknownData_set(property.vtable, property.ptr, data);
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
                Ov_Reference_free(assignation_body(&var->tuple.references[i], Ov_UnknownData_from_ptr(array.vtable, Ov_Array_get(array, i))));
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


const char separator_parameters[] = "[rr]";
Ov_Reference_Owned separator_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    return Ov_Reference_copy(args[1]);
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

            assert(Ov_UnknownData_equals(data, Ov_Reference_get(Ov_Reference_share(else_word))));
            result = Ov_Function_execute(expression->tuple.tab[i + 1]);
            data = Ov_Reference_get(Ov_Reference_share(result));

            if (Ov_UnknownData_equals(data, Ov_Reference_get(Ov_Reference_share(if_word))) && i + 3 < expression->tuple.size) {
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

const char for_statement_parameters[] = "[rc0rc1rr()]";
Ov_Reference_Owned for_statement_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_Reference_Shared variable = args[0];
    Ov_UnknownData begin = Ov_Reference_get(args[1]);
    Ov_UnknownData end = Ov_Reference_get(args[2]);
    Ov_Reference_Shared body = args[3];

    assert(begin.vtable == &Ov_VirtualTable_Int);
    assert(end.vtable == &Ov_VirtualTable_Int);

    Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
    Ov_Expression ex = {
        .type = Ov_EXPRESSION_REFERENCE,
        .reference = Ov_Reference_share(param)
    };

    for (OV_INT i = begin.data.i; i < end.data.i; ++i) {
        Ov_UnknownData new_data = {
            .vtable = &Ov_VirtualTable_Int,
            .data.i = i
        };
        Ov_Reference_Owned new_ref = Ov_Reference_new_data(new_data);
        Ov_Reference_free(Ov_set(variable, Ov_Reference_share(new_ref)));
        Ov_Reference_free(new_ref);

        Ov_Reference_Owned r = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(body)), ex);
        Ov_Reference_free(r);
    }

    Ov_Reference_free(param);
    return Ov_Reference_new_uninitialized();
}

const char for_step_statement_parameters[] = "[rc0rc1rc2rr()]";
Ov_Reference_Owned for_step_statement_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_Reference_Shared variable = args[0];
    Ov_UnknownData begin = Ov_Reference_get(args[1]);
    Ov_UnknownData end = Ov_Reference_get(args[2]);
    Ov_UnknownData s = Ov_Reference_get(args[3]);
    Ov_Reference_Shared body = args[4];

    assert(begin.vtable == &Ov_VirtualTable_Int);
    assert(end.vtable == &Ov_VirtualTable_Int);
    assert(s.vtable == &Ov_VirtualTable_Int);
    assert(s.data.i != 0);

    Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
    Ov_Expression ex = {
        .type = Ov_EXPRESSION_REFERENCE,
        .reference = Ov_Reference_share(param)
    };

    for (OV_INT i = begin.data.i; (s.data.i > 0) ? (i < end.data.i) : (i > end.data.i); ++i) {
        Ov_UnknownData new_data = {
            .vtable = &Ov_VirtualTable_Int,
            .data.i = i
        };
        Ov_Reference_Owned new_ref = Ov_Reference_new_data(new_data);
        Ov_Reference_free(Ov_set(variable, Ov_Reference_share(new_ref)));
        Ov_Reference_free(new_ref);

        Ov_Reference_Owned r = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(body)), ex);
        Ov_Reference_free(r);
    }

    Ov_Reference_free(param);
    return Ov_Reference_new_uninitialized();
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

const char function_add_parameters[] = "[rr]";
Ov_Reference_Owned function_add_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_Function function = *Ov_UnknownData_get_function(Ov_Reference_get(args[1]));

    Ov_Reference_Owned references[function->captures.size];
    size_t i;
    for (i = 0; i < function->captures.size; ++i)
        references[i] = Ov_GC_capture_to_reference(function->captures.tab[i]);

    Ov_Function* f;
    for (f = Ov_UnknownData_get_function(Ov_Reference_get(args[0])); (*f)->next != NULL; f = &(*f)->next);
    Ov_Function_push(f, function->parameters, function->body, function->filter, function->local_variables, (Ov_Reference_Shared*) references, function->captures.size);

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
        {
            unsigned a_properties = 0;
            size_t i;
            for (i = 0; i < a.vtable->table_size; ++i) {
                struct Ov_VirtualTable_Element* list;
                for (list = &a.vtable->table_tab[i]; list != NULL && list->hash != 0; list = list->next) {
                    ++a_properties;

                    Ov_PropertyInfo a_property = {
                        .vtable = list->vtable,
                        .ptr = ((BYTE*) a.data.ptr) + list->offset
                    };
                    Ov_PropertyInfo b_property = Ov_UnknownData_get_property(b, list->hash);

                    if (!equals(Ov_UnknownData_from_ptr(a_property.vtable, a_property.ptr), Ov_UnknownData_from_ptr(b_property.vtable, b_property.ptr)))
                        return false;
                }
            }
            unsigned b_properties = 0;
            for (i = 0; i < b.vtable->table_size; ++i) {
                struct Ov_VirtualTable_Element* list;
                for (list = &b.vtable->table_tab[i]; list != NULL && list->hash != 0; list = list->next) {
                    ++b_properties;
                }
            }
            if (a_properties != b_properties)
                return false;
        }

        {
            // TODO : compare functions
        }

        {
            Ov_ArrayInfo a_array = Ov_UnknownData_get_array(a);
            Ov_ArrayInfo b_array = Ov_UnknownData_get_array(b);

            if (a_array.array->size != b_array.array->size)
                return false;

            size_t i;
            for (i = 0; i < a_array.array->size; ++i)
                if (!equals(Ov_UnknownData_from_ptr(a_array.vtable, Ov_Array_get(a_array, i)), Ov_UnknownData_from_ptr(b_array.vtable, Ov_Array_get(b_array, i))))
                    return false;
        }

        return true;
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
        .data.b = Ov_UnknownData_equals(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char not_check_pointers_parameters[] = "[rr]";
Ov_Reference_Owned not_check_pointers_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = !Ov_UnknownData_equals(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
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
            sprintf(buffer, "%d", (int) data.data.i);
        else if (data.vtable == &Ov_VirtualTable_Float)
            sprintf(buffer, "%f", (float) data.data.f);
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

const char scan_parameters[] = "[]";
Ov_Reference_Owned scan_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (args);
    (void) (local_variables);

    size_t size = 64;
    char* buffer = malloc(size);
    assert(buffer != NULL);
    size_t position = 0;
    int c;
    do {
        c = fgetc(stdin);
        buffer[position++] = c;

        if (position >= size) {
            size *= 2;
            buffer = realloc(buffer, size);
            assert(buffer != NULL);
        }
    } while (c != EOF && c != '\n');
    buffer[position] = '\0';

    Ov_UnknownData string = {
        .vtable = Ov_VirtualTable_string_from_tuple,
        .data.ptr = Ov_GC_alloc_object(Ov_VirtualTable_string_from_tuple)
    };
    Ov_ArrayInfo string_array = Ov_UnknownData_get_array(string);
    Ov_Array_set_size(string_array, strlen(buffer));
    size_t i;
    for (i = 0; i < string_array.array->size; ++i) {
        Ov_UnknownData c = {
            .vtable = &Ov_VirtualTable_Char,
            .data.c = buffer[i]
        };
        Ov_UnknownData_set(string_array.vtable, Ov_Array_get(string_array, i), c);
    }

    return Ov_Reference_new_data(string);
}


Ov_Reference_Owned getter;
Ov_Reference_Owned function_getter;
Ov_Reference_Owned defined;
Ov_Reference_Owned setter;
Ov_Reference_Owned _x3A_x3D;
Ov_Reference_Owned _x3B;
Ov_Reference_Owned if_statement;
Ov_Reference_Owned else_statement;
Ov_Reference_Owned while_statement;
Ov_Reference_Owned for_statement;
Ov_Reference_Owned from;
Ov_Reference_Owned to;
Ov_Reference_Owned for_step_statement;
Ov_Reference_Owned step;
Ov_Reference_Owned _x24;
Ov_Reference_Owned _x24_x3D_x3D;
Ov_Reference_Owned _x3D;
Ov_Reference_Owned _x3A;
Ov_Reference_Owned _x7C;
Ov_Reference_Owned _x3D_x3D;
Ov_Reference_Owned _x21_x3D;
Ov_Reference_Owned _x3D_x3D_x3D;
Ov_Reference_Owned _x21_x3D_x3D;
Ov_Reference_Owned string_from;
Ov_Reference_Owned print;
Ov_Reference_Owned scan;

void Ov_init_functions_base() {
    getter = Ov_Reference_new_uninitialized();
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(getter))), getter_parameters, getter_body, getter_filter, 1, NULL, 0);

    function_getter = Ov_Reference_new_uninitialized();
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(function_getter))), function_getter_parameters, function_getter_body, NULL, 0, NULL, 0);

    defined = Ov_Reference_new_uninitialized();
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(defined))), defined_parameters, defined_body, NULL, 0, NULL, 0);

    setter = Ov_Reference_new_uninitialized();
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(setter))), setter_parameters, setter_body, NULL, 0, NULL, 0);
    _x3A_x3D = Ov_Reference_new_uninitialized();
    Ov_set(Ov_Reference_share(_x3A_x3D), Ov_Reference_share(setter));

    Ov_UnknownData separator_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(separator_data), separator_parameters, separator_body, NULL, 0, NULL, 0);
    _x3B = Ov_Reference_new_symbol(separator_data);

    Ov_UnknownData else_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    else_statement = Ov_Reference_new_symbol(else_statement_data);
    Ov_UnknownData if_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    if_statement = Ov_Reference_new_symbol(if_statement_data);
    Ov_Reference_Shared if_statement_captures[] = {
        Ov_Reference_share(if_statement),
        Ov_Reference_share(else_statement)
    };
    Ov_Function_push(Ov_UnknownData_get_function(if_statement_data), if_statement_parameters, if_statement_body, NULL, 0, if_statement_captures, sizeof(if_statement_captures) / sizeof(if_statement_captures[0]));

    Ov_UnknownData while_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(while_statement_data), while_statement_parameters, while_statement_body, NULL, 0, NULL, 0);
    while_statement = Ov_Reference_new_symbol(while_statement_data);

    Ov_UnknownData from_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    from = Ov_Reference_new_symbol(from_data);
    Ov_UnknownData to_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    to = Ov_Reference_new_symbol(to_data);
    Ov_Reference_Shared for_statement_captures[] = {
        Ov_Reference_share(from),
        Ov_Reference_share(to)
    };
    Ov_UnknownData for_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(for_statement_data), for_statement_parameters, for_statement_body, NULL, 0, for_statement_captures, sizeof(for_statement_captures) / sizeof(for_statement_captures[0]));
    for_statement = Ov_Reference_new_symbol(for_statement_data);

    Ov_UnknownData step_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    step = Ov_Reference_new_symbol(step_data);
    Ov_Reference_Shared for_step_statement_captures[] = {
        Ov_Reference_share(from),
        Ov_Reference_share(to),
        Ov_Reference_share(step)
    };
    Ov_UnknownData for_step_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(for_step_statement_data), for_step_statement_parameters, for_step_statement_body, NULL, 0, for_step_statement_captures, sizeof(for_step_statement_captures) / sizeof(for_step_statement_captures[0]));
    for_step_statement = Ov_Reference_new_symbol(for_step_statement_data);

    Ov_UnknownData copy_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(copy_data), copy_parameters, copy_body, NULL, 0, NULL, 0);
    _x24 = Ov_Reference_new_symbol(copy_data);

    Ov_UnknownData copy_pointer_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(copy_pointer_data), copy_pointer_parameters, copy_pointer_body, NULL, 0, NULL, 0);
    _x24_x3D_x3D = Ov_Reference_new_symbol(copy_pointer_data);

    Ov_UnknownData define_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(define_data), setter_parameters, setter_body, setter_filter, 0, NULL, 0);
    _x3D = Ov_Reference_new_symbol(define_data);

    Ov_UnknownData function_definition_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(function_definition_data), function_definition_parameters, function_definition_body, NULL, 0, NULL, 0);
    _x3A = Ov_Reference_new_symbol(function_definition_data);

    Ov_UnknownData function_add_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(function_add_data), function_add_parameters, function_add_body, NULL, 0, NULL, 0);
    _x7C = Ov_Reference_new_symbol(function_add_data);

    Ov_UnknownData equals_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(equals_data), equals_parameters, equals_body, NULL, 0, NULL, 0);
    _x3D_x3D = Ov_Reference_new_symbol(equals_data);

    Ov_UnknownData not_equals_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(not_equals_data), not_equals_parameters, not_equals_body, NULL, 0, NULL, 0);
    _x21_x3D = Ov_Reference_new_symbol(not_equals_data);

    Ov_UnknownData check_pointers_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(check_pointers_data), check_pointers_parameters, check_pointers_body, NULL, 0, NULL, 0);
    _x3D_x3D_x3D = Ov_Reference_new_symbol(check_pointers_data);

    Ov_UnknownData not_check_pointers_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(not_check_pointers_data), not_check_pointers_parameters, not_check_pointers_body, NULL, 0, NULL, 0);
    _x21_x3D_x3D = Ov_Reference_new_symbol(not_check_pointers_data);

    Ov_UnknownData string_from_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(string_from_data), string_from_parameters, string_from_body, NULL, 0, NULL, 0);
    string_from = Ov_Reference_new_symbol(string_from_data);

    Ov_UnknownData print_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(print_data), print_parameters, print_body, NULL, 0, NULL, 0);
    print = Ov_Reference_new_symbol(print_data);

    Ov_UnknownData scan_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(scan_data), scan_parameters, scan_body, NULL, 0, NULL, 0);
    scan = Ov_Reference_new_symbol(scan_data);
}
