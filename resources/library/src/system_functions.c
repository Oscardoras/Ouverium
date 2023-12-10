#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "include.h"


const char Ov_system_function_getter_parameters[] = "r";
Ov_Reference_Owned Ov_system_function_getter_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);

    Ov_GC_Reference* reference = (Ov_GC_Reference*)args[0];
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

const char Ov_system_function_defined_parameters[] = "r";
Ov_Reference_Owned Ov_system_function_defined_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    bool defined = true;

    Ov_GC_Reference* reference = (Ov_GC_Reference*)args[0];
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

const char Ov_system_function_setter_parameters[] = "[rr]";
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

    return Ov_Reference_copy((Ov_Reference_Shared)var);
}
Ov_Reference_Owned Ov_system_function_setter_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    return assignation_body((Ov_GC_Reference*)args[0], Ov_Reference_get(args[1]));
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
        }
        else return false;
    }
    else return true;
}
bool Ov_system_function_setter_filter(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    return assignation_filter((Ov_GC_Reference*)args[0], Ov_Reference_get(args[1]));
}

const char Ov_system_function_separator_parameters[] = "r";
Ov_Reference_Owned Ov_system_function_separator_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    return Ov_Reference_get_element(args[0], Ov_Reference_get_size(args[0]) - 1);
}

Ov_Reference_Owned Ov_system_function_if_statement_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    //TODO
    return;
}

const char Ov_system_function_while_statement_parameters[] = "[r()r()]";
Ov_Reference_Owned Ov_system_function_while_statement_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);

    Ov_Reference_Owned result = Ov_Reference_new_uninitialized();

    Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
    Ov_Expression ex = {
        .type = Ov_EXPRESSION_REFERENCE,
        .reference = Ov_Reference_share(param)
    };
    while (true) {
        Ov_Reference_Owned c = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), ex);
        Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(c));

        if (data.vtable == &Ov_VirtualTable_Bool) {
            if (data.data.b) {
                Ov_Reference_Owned r = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), ex);
                Ov_Reference_free(result);
                result = r;
            } else break;
        } else {
            // TODO : throw exception
        }

        Ov_Reference_free(c);
    }

    Ov_Reference_free(param);
    return result;
}

const char Ov_system_function_copy_parameters[] = "r";
bool Ov_system_function_copy_filter(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    Ov_VirtualTable* vtable = Ov_Reference_get(args[0]).vtable;

    return
        vtable == &Ov_VirtualTable_Int ||
        vtable == &Ov_VirtualTable_Float ||
        vtable == &Ov_VirtualTable_Char ||
        vtable == &Ov_VirtualTable_Bool;
}
Ov_Reference_Owned Ov_system_function_copy_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    Ov_UnknownData data = Ov_Reference_get(args[0]);

    return Ov_Reference_new_data(data);
}

const char Ov_system_function_copy_pointer_parameters[] = "r";
Ov_Reference_Owned Ov_system_function_copy_pointer_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    Ov_UnknownData data = Ov_Reference_get(args[0]);

    return Ov_Reference_new_data(data);
}

const char Ov_system_function_function_definition_parameters[] = "[rr]";
Ov_Reference_Owned Ov_system_function_function_definition_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
    Ov_Function function = *Ov_UnknownData_get_function(Ov_Reference_get(args[1]));

    Ov_Reference_Owned references[function->captures.size];
    size_t i;
    for (i = 0; i < function->captures.size; ++i)
        references[i] = Ov_GC_capture_to_reference(function->captures.tab[i]);
    Ov_Function_push(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), function->parameters, function->body, function->filter, references, function->captures.size);

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

const char Ov_system_function_equals_parameters[] = "[rr]";
Ov_Reference_Owned Ov_system_function_equals_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = equals(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char Ov_system_function_not_equals_parameters[] = "[rr]";
Ov_Reference_Owned Ov_system_function_not_equals_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = !equals(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

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

const char Ov_system_function_check_pointers_parameters[] = "[rr]";
Ov_Reference_Owned Ov_system_function_check_pointers_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = check_pointers(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char Ov_system_function_not_check_pointers_parameters[] = "[rr]";
Ov_Reference_Owned Ov_system_function_not_check_pointers_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = !check_pointers(Ov_Reference_get(args[0]), Ov_Reference_get(args[1]))
    };
    return Ov_Reference_new_data(data);
}

const char Ov_system_function_string_from_parameters[] = "[rr]";
extern Ov_VirtualTable* Ov_VirtualTable_string_from_tuple;
Ov_Reference_Owned Ov_system_function_string_from_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
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

            Ov_Array_set_size(string_array, string_array.array->size + str_array.array->size);
            size_t j;
            for (j = 0; j < str_array.array->size; ++j)
                Ov_UnknownData_set(
                    string_array.vtable,
                    Ov_Array_get(string_array, string_array.array->size + j),
                    Ov_UnknownData_from_ptr(str_array.vtable, Ov_Array_get(str_array, j))
                );

            Ov_Reference_free(str);
            Ov_Reference_free(r);
        }
    }

    return Ov_Reference_new_data(string);
}

const char Ov_system_function_print_parameters[] = "r";
Ov_Reference_Owned Ov_system_function_print_body(Ov_Reference_Owned captures[], Ov_Reference_Shared args[]) {
    (void)(captures);
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


void Ov_init_functions() {
    Ov_UnknownData getter_data = {
        .vtable = &Ov_VirtualTable_Function,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Function)
    };
    Ov_Function_push(Ov_UnknownData_get_function(getter_data), Ov_system_function_separator_parameters, Ov_system_function_separator_body, NULL, NULL, 0);
    getter = Ov_Reference_new_symbol(getter_data);
}
