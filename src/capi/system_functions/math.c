#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <ouverium/include.h>


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


Ov_Reference_Owned _x21;
Ov_Reference_Owned _x26;
Ov_Reference_Owned _x7C;
Ov_Reference_Owned _x2B;
Ov_Reference_Owned _x2D;
Ov_Reference_Owned _x2A;
Ov_Reference_Owned _x2F;
Ov_Reference_Owned _x25;

void Ov_init_functions_math() {
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
