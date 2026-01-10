#include <assert.h>

#include <ouverium/hash_string.h>
#include <ouverium/include.h>

#include "../gc.h"


const char constructor_parameters[] = "r";

Ov_Reference_Owned constructor_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    return Ov_Reference_copy(args[0]);
}

bool char_constructor_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Char;
}

bool float_constructor_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Float;
}

bool int_constructor_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Int;
}

bool bool_constructor_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Bool;
}

bool array_constructor_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    Ov_ArrayInfo array = Ov_UnknownData_get_array(a);
    return array.vtable != NULL && array.array->capacity > 0;
}

bool function_constructor_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable->function.offset >= 0 && *Ov_UnknownData_get_function(a) != NULL;
}

const char is_type_parameters[] = "[rr]";
bool is_type_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData type = Ov_Reference_get(args[1]);

    if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Char)).data.ptr)
        return true;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Float)).data.ptr)
        return true;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Int)).data.ptr)
        return true;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Bool)).data.ptr)
        return true;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Array)).data.ptr)
        return true;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Function)).data.ptr)
        return true;

    return false;
}
Ov_Reference_Owned is_type_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData type = Ov_Reference_get(args[1]);

    bool value = false;
    if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Char)).data.ptr)
        value = a.vtable == &Ov_VirtualTable_Char;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Float)).data.ptr)
        value = a.vtable == &Ov_VirtualTable_Float;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Int)).data.ptr)
        value = a.vtable == &Ov_VirtualTable_Int;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Bool)).data.ptr)
        value = a.vtable == &Ov_VirtualTable_Bool;
    else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Array)).data.ptr) {
        Ov_ArrayInfo array = Ov_UnknownData_get_array(a);
        value = array.vtable != NULL && array.array->capacity > 0;
    } else if (type.data.ptr == Ov_Reference_get(Ov_Reference_share(Function)).data.ptr) {
        value = a.vtable->function.offset >= 0 && *Ov_UnknownData_get_function(a) != NULL;
    }

    Ov_UnknownData data = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = value
    };
    return Ov_Reference_new_data(data);
}


void Ov_init_functions_types(void) {
    Ov_Function_push(Ov_get_function(Ov_Reference_share(Char)), constructor_parameters, constructor_body, char_constructor_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(Float)), constructor_parameters, constructor_body, float_constructor_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(Int)), constructor_parameters, constructor_body, int_constructor_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(Bool)), constructor_parameters, constructor_body, bool_constructor_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(Array)), constructor_parameters, constructor_body, array_constructor_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(Function)), constructor_parameters, constructor_body, function_constructor_filter, 0, NULL, 0);

    Ov_Function_push(Ov_get_function(Ov_Reference_share(_x7E)), is_type_parameters, is_type_body, is_type_filter, 0, NULL, 0);
}
