#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ouverium/include.h>

#include <ouverium/types.h>

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
            Ov_UnknownData_set(&Ov_VirtualTable_UnknownData, var->symbol, data);
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

    if (Ov_Reference_raw(args[0]).vtable == NULL) {
        Ov_UnknownData data = {
            .vtable = &Ov_VirtualTable_Object,
            .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
        };

        Ov_Reference_free(assignation_body((Ov_GC_Reference*) args[0], data));
    }

    Ov_Function* a = Ov_UnknownData_get_function(Ov_Reference_get(args[0]));
    Ov_Function b = *Ov_UnknownData_get_function(Ov_Reference_get(args[1]));

    Ov_Function cpy = Ov_Function_copy(b);

    struct Ov_FunctionCell** f_ptr;
    for (f_ptr = &cpy; *f_ptr != NULL; f_ptr = &(*f_ptr)->next);

    *f_ptr = *a;
    *a = cpy;

    return Ov_Reference_copy(args[0]);
}

const char function_add_parameters[] = "[rr]";
Ov_Reference_Owned function_add_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    if (Ov_Reference_raw(args[0]).vtable == NULL) {
        Ov_UnknownData data = {
            .vtable = &Ov_VirtualTable_Object,
            .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
        };

        Ov_Reference_free(assignation_body((Ov_GC_Reference*) args[0], data));
    }

    Ov_Function* a = Ov_UnknownData_get_function(Ov_Reference_get(args[0]));
    Ov_Function f = *Ov_UnknownData_get_function(Ov_Reference_get(args[1]));

    Ov_Function cpy = Ov_Function_copy(f);

    struct Ov_FunctionCell** f_ptr;
    for (f_ptr = a; *f_ptr != NULL; f_ptr = &(*f_ptr)->next);

    *f_ptr = cpy;

    return Ov_Reference_copy(args[0]);
}

static bool equals(Ov_UnknownData a, Ov_UnknownData b) {
    if (a.vtable == &Ov_VirtualTable_Char) {
        if (b.vtable == a.vtable)
            return a.data.c == b.data.c;
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


void Ov_init_functions_base() {
    Ov_Function_push(Ov_get_function(Ov_Reference_share(getter)), getter_parameters, getter_body, getter_filter, 1, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(function_getter)), function_getter_parameters, function_getter_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(defined)), defined_parameters, defined_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(setter)), setter_parameters, setter_body, NULL, 0, NULL, 0);
    Ov_set(Ov_Reference_share(_x3A_x3D), Ov_Reference_share(setter));

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x3B)), separator_parameters, separator_body, NULL, 0, NULL, 0);

    Ov_UnknownData else_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    else_keyword = Ov_Reference_new_symbol(else_statement_data);
    Ov_UnknownData if_statement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    if_keyword = Ov_Reference_new_symbol(if_statement_data);
    Ov_Reference_Shared if_statement_captures[] = {
        Ov_Reference_share(if_keyword),
        Ov_Reference_share(else_keyword)
    };
    Ov_Function_push(Ov_get_function(Ov_Reference_share(if_keyword)), if_statement_parameters, if_statement_body, NULL, 0, if_statement_captures, sizeof(if_statement_captures) / sizeof(if_statement_captures[0]));

    Ov_Function_push(Ov_get_function(Ov_Reference_share(while_keyword)), while_statement_parameters, while_statement_body, NULL, 0, NULL, 0);

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
    for_keyword = Ov_Reference_new_symbol(for_statement_data);
    Ov_Function_push(Ov_get_function(Ov_Reference_share(for_keyword)), for_statement_parameters, for_statement_body, NULL, 0, for_statement_captures, sizeof(for_statement_captures) / sizeof(for_statement_captures[0]));

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
    Ov_Function_push(Ov_get_function(Ov_Reference_share(for_keyword)), for_step_statement_parameters, for_step_statement_body, NULL, 0, for_step_statement_captures, sizeof(for_step_statement_captures) / sizeof(for_step_statement_captures[0]));

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x24)), copy_parameters, copy_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x24_x3D_x3D)), copy_pointer_parameters, copy_pointer_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x3D)), setter_parameters, setter_body, setter_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x3A)), function_definition_parameters, function_definition_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x7C)), function_add_parameters, function_add_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x3D_x3D)), equals_parameters, equals_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x21_x3D)), not_equals_parameters, not_equals_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x3D_x3D_x3D)), check_pointers_parameters, check_pointers_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x21_x3D_x3D)), not_check_pointers_parameters, not_check_pointers_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(string_from)), string_from_parameters, string_from_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(print)), print_parameters, print_body, NULL, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(scan)), scan_parameters, scan_body, NULL, 0, NULL, 0);
}
