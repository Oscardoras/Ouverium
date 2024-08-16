#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <ouverium/hash_string.h>
#include <ouverium/include.h>

#include "../gc.h"


const char get_capacity_parameters[] = "r";
bool get_capacity_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);

    return Ov_UnknownData_get_array(array).vtable != NULL;
}
Ov_Reference_Owned get_capacity_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);

    Ov_UnknownData capacity = {
        .vtable = &Ov_VirtualTable_Int,
        .data.i = Ov_UnknownData_get_array(array).array->capacity
    };
    return Ov_Reference_new_data(capacity);
}

const char set_capacity_parameters[] = "[rr]";
bool set_capacity_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);
    Ov_UnknownData capacity = Ov_Reference_get(args[1]);

    return Ov_UnknownData_get_array(array).vtable != NULL && capacity.vtable == &Ov_VirtualTable_Int;
}
Ov_Reference_Owned set_capacity_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);
    Ov_UnknownData capacity = Ov_Reference_get(args[1]);

    Ov_Array_set_capacity(Ov_UnknownData_get_array(array), (size_t) capacity.data.i);

    return Ov_Reference_new_uninitialized();
}

const char get_size_parameters[] = "r";
bool get_size_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);

    return Ov_UnknownData_get_array(array).vtable != NULL;
}
Ov_Reference_Owned get_size_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);

    Ov_UnknownData capacity = {
        .vtable = &Ov_VirtualTable_Int,
        .data.i = Ov_UnknownData_get_array(array).array->size
    };
    return Ov_Reference_new_data(capacity);
}

const char set_size_parameters[] = "[rr]";
bool set_size_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);
    Ov_UnknownData size = Ov_Reference_get(args[1]);

    return Ov_UnknownData_get_array(array).vtable != NULL && size.vtable == &Ov_VirtualTable_Int;
}
Ov_Reference_Owned set_size_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);
    Ov_UnknownData size = Ov_Reference_get(args[1]);

    Ov_Array_set_size(Ov_UnknownData_get_array(array), (size_t) size.data.i);

    return Ov_Reference_new_uninitialized();
}

const char get_parameters[] = "[rr]";
bool get_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);
    Ov_UnknownData i = Ov_Reference_get(args[1]);

    return Ov_UnknownData_get_array(array).vtable != NULL && i.vtable == &Ov_VirtualTable_Int
        && i.data.i >= 0 && i.data.i < (OV_INT) Ov_UnknownData_get_array(array).array->size;
}
Ov_Reference_Owned get_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData array = Ov_Reference_get(args[0]);
    Ov_UnknownData i = Ov_Reference_get(args[1]);

    return Ov_Reference_new_array(array, i.data.i);
}

const char copy_data_parameters[] = "[rrrrr]";
bool copy_data_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_ArrayInfo from_array = Ov_UnknownData_get_array(Ov_Reference_get(args[0]));
    OV_INT from_i = Ov_Reference_get(args[1]).data.i;
    Ov_ArrayInfo to_array = Ov_UnknownData_get_array(Ov_Reference_get(args[2]));
    OV_INT to_i = Ov_Reference_get(args[3]).data.i;
    OV_INT n = Ov_Reference_get(args[4]).data.i;

    if (n < 0)
        return false;

    if (from_i < 0 || (size_t) from_i + n > from_array.array->size)
        return false;
    if (to_i < 0 || (size_t) to_i + n > to_array.array->size)
        return false;

    return true;
}
Ov_Reference_Owned copy_data_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_ArrayInfo from_array = Ov_UnknownData_get_array(Ov_Reference_get(args[0]));
    OV_INT from_i = Ov_Reference_get(args[1]).data.i;
    Ov_ArrayInfo to_array = Ov_UnknownData_get_array(Ov_Reference_get(args[2]));
    OV_INT to_i = Ov_Reference_get(args[3]).data.i;
    OV_INT n = Ov_Reference_get(args[4]).data.i;

    if (n != 0) {
        if (from_i < to_i) {
            for (OV_INT i = n - 1; i >= 0; --i)
                Ov_UnknownData_set(
                    to_array.vtable, Ov_Array_get(to_array, (size_t) to_i + i),
                    Ov_UnknownData_from_ptr(from_array.vtable, from_array.array->tab + (size_t) from_i + i)
                );
        } else {
            for (OV_INT i = 0; i < n; ++i)
                Ov_UnknownData_set(
                    to_array.vtable, Ov_Array_get(to_array, (size_t) to_i + i),
                    Ov_UnknownData_from_ptr(from_array.vtable, from_array.array->tab + (size_t) from_i + i)
                );
        }
    }

    return Ov_Reference_new_uninitialized();
}

const char foreach_parameters[] = "[r()r]";
bool foreach_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (args);
    (void) (local_variables);

    // TODO

    return true;
}
Ov_Reference_Owned foreach_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);

    Ov_Reference_Owned param = Ov_Reference_new_uninitialized();
    Ov_Expression ex = {
        .type = Ov_EXPRESSION_REFERENCE,
        .reference = Ov_Reference_share(param)
    };
    Ov_GC_Reference* array = (Ov_GC_Reference*) Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), ex);

    if (array->type == TUPLE) {
        size_t i;
        for (i = 0; i < array->tuple.size; ++i) {
            Ov_Expression expr = {
                .type = Ov_EXPRESSION_REFERENCE,
                .reference = Ov_Reference_share(&array->tuple.references[i])
            };
            Ov_Reference_Owned r = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), expr);
            Ov_Reference_free(r);
        }
    } else {
        Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(array));
        Ov_ArrayInfo array_info = Ov_UnknownData_get_array(data);
        size_t i;
        for (i = 0; i < array_info.array->size; ++i) {
            Ov_Reference_Owned array_param = Ov_Reference_new_array(data, i);
            Ov_Expression expr = {
                .type = Ov_EXPRESSION_REFERENCE,
                .reference = Ov_Reference_share(array_param)
            };
            Ov_Reference_Owned r = Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(args[0])), expr);
            Ov_Reference_free(r);
            Ov_Reference_free(array_param);
        }
    }

    Ov_Reference_free((Ov_Reference_Owned) array);
    return Ov_Reference_new_uninitialized();
}


Ov_Reference_Owned Array;


void Ov_init_functions_array() {
    Array = Ov_Reference_new_uninitialized();
    Ov_UnknownData Array_data = Ov_get_object(Ov_Reference_share(Array));

    Ov_Reference_Owned get_capacity = Ov_Reference_new_property(Array_data, hash_string("get_capacity"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(get_capacity))), get_capacity_parameters, get_capacity_body, get_capacity_filter, 0, NULL, 0);
    Ov_Reference_free(get_capacity);

    Ov_Reference_Owned set_capacity = Ov_Reference_new_property(Array_data, hash_string("set_capacity"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(set_capacity))), set_capacity_parameters, set_capacity_body, set_capacity_filter, 0, NULL, 0);
    Ov_Reference_free(set_capacity);

    Ov_Reference_Owned get_size = Ov_Reference_new_property(Array_data, hash_string("get_size"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(get_size))), get_size_parameters, get_size_body, get_size_filter, 0, NULL, 0);
    Ov_Reference_free(get_size);

    Ov_Reference_Owned set_size = Ov_Reference_new_property(Array_data, hash_string("set_size"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(set_size))), set_size_parameters, set_size_body, set_size_filter, 0, NULL, 0);
    Ov_Reference_free(set_size);

    Ov_Reference_Owned get = Ov_Reference_new_property(Array_data, hash_string("get"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(get))), get_parameters, get_body, get_filter, 0, NULL, 0);
    Ov_Reference_free(get);

    Ov_Reference_Owned copy_data = Ov_Reference_new_property(Array_data, hash_string("copy_data"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(copy_data))), copy_data_parameters, copy_data_body, copy_data_filter, 0, NULL, 0);
    Ov_Reference_free(copy_data);

    Ov_Reference_Owned foreach = Ov_Reference_new_property(Array_data, hash_string("foreach"));
    Ov_Function_push(Ov_UnknownData_get_function(Ov_get_object(Ov_Reference_share(foreach))), foreach_parameters, foreach_body, foreach_filter, 0, NULL, 0);
    Ov_Reference_free(foreach);
}
