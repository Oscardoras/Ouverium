#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <ouverium/include.h>

#include "../gc.h"


const char a_parameters[] = "r";
const char ab_parameters[] = "[rr]";
const char a_b_parameters[] = "[rr()]";

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

bool logical_and_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b_lambda = Ov_Reference_get(args[1]);

    if (a.vtable == &Ov_VirtualTable_Bool) {
        if (a.data.b) {
            Ov_Expression expr = {
                .type = Ov_EXPRESSION_TUPLE,
                .tuple = {
                    .vtable = &Ov_VirtualTable_Object,
                    .size = 0,
                    .tab = NULL
                }
            };
            Ov_Reference_Owned c = Ov_Function_eval(Ov_UnknownData_get_function(b_lambda), expr);
            Ov_UnknownData b = Ov_Reference_get(Ov_Reference_share(c));

            if (b.vtable == &Ov_VirtualTable_Bool) {
                ((Ov_GC_Reference*) local_variables[0])->data = b;

                Ov_Reference_free(c);
                return true;
            } else {
                Ov_Reference_free(c);
                return false;
            }
        } else {
            ((Ov_GC_Reference*) local_variables[0])->data = a;
            return true;
        }
    } else return false;
}
Ov_Reference_Owned logical_and_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (args);

    return Ov_Reference_new_data(local_variables[0]->data);
}

bool logical_or_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b_lambda = Ov_Reference_get(args[1]);

    if (a.vtable == &Ov_VirtualTable_Bool) {
        if (a.data.b) {
            ((Ov_GC_Reference*) local_variables[0])->data = a;
            return true;
        } else {
            Ov_Expression expr = {
                .type = Ov_EXPRESSION_TUPLE,
                .tuple = {
                    .vtable = &Ov_VirtualTable_Object,
                    .size = 0,
                    .tab = NULL
                }
            };
            Ov_Reference_Owned c = Ov_Function_eval(Ov_UnknownData_get_function(b_lambda), expr);
            Ov_UnknownData b = Ov_Reference_get(Ov_Reference_share(c));

            if (b.vtable == &Ov_VirtualTable_Bool) {
                ((Ov_GC_Reference*) local_variables[0])->data = b;

                Ov_Reference_free(c);
                return true;
            } else {
                Ov_Reference_free(c);
                return false;
            }
        }
    } else return false;
}
Ov_Reference_Owned logical_or_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (args);

    return Ov_Reference_new_data(local_variables[0]->data);
}

bool addition_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
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

bool opposite_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float;
}
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

bool substraction_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
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

bool multiplication_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
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

bool division_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
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

bool modulo_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return a.vtable == &Ov_VirtualTable_Int && b.vtable == &Ov_VirtualTable_Int;
}
Ov_Reference_Owned modulo_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    Ov_UnknownData r = {
        .vtable = &Ov_VirtualTable_Int,
        .data.i = a.data.i % b.data.i
    };
    return Ov_Reference_new_data(r);
}

bool strictly_inf_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned strictly_inf_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    bool v;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.i < b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.i < b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.f < b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.f < b.data.f;
        }
    }

    Ov_UnknownData r = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = v
    };
    return Ov_Reference_new_data(r);
}

bool strictly_sup_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned strictly_sup_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    bool v;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.i > b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.i > b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.f > b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.f > b.data.f;
        }
    }

    Ov_UnknownData r = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = v
    };
    return Ov_Reference_new_data(r);
}

bool inf_equals_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned inf_equals_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    bool v;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.i <= b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.i <= b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.f <= b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.f <= b.data.f;
        }
    }

    Ov_UnknownData r = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = v
    };
    return Ov_Reference_new_data(r);
}

bool sup_equals_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned sup_equals_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    bool v;
    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.i >= b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.i >= b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            v = a.data.f >= b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            v = a.data.f >= b.data.f;
        }
    }

    Ov_UnknownData r = {
        .vtable = &Ov_VirtualTable_Bool,
        .data.b = v
    };
    return Ov_Reference_new_data(r);
}

bool increment_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Int;
}
Ov_Reference_Owned increment_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    Ov_UnknownData new_data = {
        .vtable = &Ov_VirtualTable_Int,
        .data.i = a.data.i + 1
    };
    Ov_Reference_Owned new_ref = Ov_Reference_new_data(new_data);
    Ov_Reference_Owned ref = Ov_set(args[0], Ov_Reference_share(new_ref));
    Ov_Reference_free(new_ref);

    return ref;
}

bool decrement_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    return a.vtable == &Ov_VirtualTable_Int;
}
Ov_Reference_Owned decrement_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);

    Ov_UnknownData new_data = {
        .vtable = &Ov_VirtualTable_Int,
        .data.i = a.data.i - 1
    };
    Ov_Reference_Owned new_ref = Ov_Reference_new_data(new_data);
    Ov_Reference_Owned ref = Ov_set(args[0], Ov_Reference_share(new_ref));
    Ov_Reference_free(new_ref);

    return ref;
}

bool add_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned add_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.i = a.data.i + b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.i + b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.f = a.data.f + b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.f + b.data.f;
        }
    }

    Ov_Reference_Owned ref = Ov_Reference_new_data(a);
    Ov_Reference_Owned r = Ov_set(args[0], Ov_Reference_share(ref));
    Ov_Reference_free(ref);
    return r;
}

bool remove_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned remove_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.i = a.data.i - b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.i - b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.f = a.data.f - b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.f - b.data.f;
        }
    }

    Ov_Reference_Owned ref = Ov_Reference_new_data(a);
    Ov_Reference_Owned r = Ov_set(args[0], Ov_Reference_share(ref));
    Ov_Reference_free(ref);
    return r;
}

bool multiply_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned multiply_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.i = a.data.i * b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.i * b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.f = a.data.f * b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.f * b.data.f;
        }
    }

    Ov_Reference_Owned ref = Ov_Reference_new_data(a);
    Ov_Reference_Owned r = Ov_set(args[0], Ov_Reference_share(ref));
    Ov_Reference_free(ref);
    return r;
}

bool divide_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    return (a.vtable == &Ov_VirtualTable_Int || a.vtable == &Ov_VirtualTable_Float) && (b.vtable == &Ov_VirtualTable_Int || b.vtable == &Ov_VirtualTable_Float);
}
Ov_Reference_Owned divide_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
    (void) (captures);
    (void) (local_variables);
    Ov_UnknownData a = Ov_Reference_get(args[0]);
    Ov_UnknownData b = Ov_Reference_get(args[1]);

    if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.i = a.data.i / b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.i / b.data.f;
        }
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == &Ov_VirtualTable_Int) {
            a.data.f = a.data.f / b.data.i;
        } else if (b.vtable == &Ov_VirtualTable_Float) {
            a.data.f = a.data.f / b.data.f;
        }
    }

    Ov_Reference_Owned ref = Ov_Reference_new_data(a);
    Ov_Reference_Owned r = Ov_set(args[0], Ov_Reference_share(ref));
    Ov_Reference_free(ref);
    return r;
}

// const char for_parameters[] = "[r()r]";

// Ov_Reference_Owned forall_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {
//     (void) (captures);
//     (void) (local_variables);
//     Ov_UnknownData a = Ov_Reference_get(args[0]);
//     Ov_UnknownData b = Ov_Reference_get(args[1]);

//     if (a.vtable == &Ov_VirtualTable_Int) {
//         if (b.vtable == &Ov_VirtualTable_Int) {
//             a.data.i = a.data.i / b.data.i;
//         } else if (b.vtable == &Ov_VirtualTable_Float) {
//             a.data.f = a.data.i / b.data.f;
//         }
//     } else if (a.vtable == &Ov_VirtualTable_Float) {
//         if (b.vtable == &Ov_VirtualTable_Int) {
//             a.data.f = a.data.f / b.data.i;
//         } else if (b.vtable == &Ov_VirtualTable_Float) {
//             a.data.f = a.data.f / b.data.f;
//         }
//     }

//     Ov_Reference_Owned ref = Ov_Reference_new_data(a);
//     Ov_Reference_Owned r = Ov_set(args[0], Ov_Reference_share(ref));
//     Ov_Reference_free(ref);
//     return r;
// }


Ov_Reference_Owned _x21;
Ov_Reference_Owned _x26;
Ov_Reference_Owned _x2B;
Ov_Reference_Owned _x2D;
Ov_Reference_Owned _x2A;
Ov_Reference_Owned _x2F;
Ov_Reference_Owned _x25;
Ov_Reference_Owned _x3C;
Ov_Reference_Owned _x3E;
Ov_Reference_Owned _x3C_x3D;
Ov_Reference_Owned _x3E_x3D;
Ov_Reference_Owned _x2B_x2B;
Ov_Reference_Owned _x2D_x2D;
Ov_Reference_Owned _x3A_x2B_x3D;
Ov_Reference_Owned _x3A_x2D_x3D;
Ov_Reference_Owned _x3A_x2A_x3D;
Ov_Reference_Owned _x3A_x2F_x3D;

void Ov_init_functions_math() {
    Ov_UnknownData logical_not_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(logical_not_data), a_parameters, logical_not_body, logical_not_filter, 0, NULL, 0);
    _x21 = Ov_Reference_new_symbol(logical_not_data);

    Ov_UnknownData logical_and_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(logical_and_data), a_b_parameters, logical_and_body, logical_and_filter, 1, NULL, 0);
    _x26 = Ov_Reference_new_symbol(logical_and_data);

    Ov_UnknownData logical_or_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(logical_or_data), a_b_parameters, logical_or_body, logical_or_filter, 1, NULL, 0);
    _x7C = Ov_Reference_new_symbol(logical_or_data);

    Ov_UnknownData addition_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(addition_data), ab_parameters, addition_body, addition_filter, 0, NULL, 0);
    _x2B = Ov_Reference_new_symbol(addition_data);

    Ov_UnknownData minus_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(minus_data), ab_parameters, opposite_body, opposite_filter, 0, NULL, 0);
    Ov_Function_push(Ov_UnknownData_get_function(minus_data), ab_parameters, substraction_body, substraction_filter, 0, NULL, 0);
    _x2D = Ov_Reference_new_symbol(minus_data);

    Ov_UnknownData multiplication_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(multiplication_data), ab_parameters, multiplication_body, multiplication_filter, 0, NULL, 0);
    _x2A = Ov_Reference_new_symbol(multiplication_data);

    Ov_UnknownData division_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(division_data), ab_parameters, division_body, division_filter, 0, NULL, 0);
    _x2F = Ov_Reference_new_symbol(division_data);

    Ov_UnknownData modulo_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(modulo_data), ab_parameters, modulo_body, modulo_filter, 0, NULL, 0);
    _x25 = Ov_Reference_new_symbol(modulo_data);

    Ov_UnknownData strictly_inf_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(strictly_inf_data), ab_parameters, strictly_inf_body, strictly_inf_filter, 0, NULL, 0);
    _x3C = Ov_Reference_new_symbol(strictly_inf_data);

    Ov_UnknownData strictly_sup_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(strictly_sup_data), ab_parameters, strictly_sup_body, strictly_sup_filter, 0, NULL, 0);
    _x3E = Ov_Reference_new_symbol(strictly_sup_data);

    Ov_UnknownData inf_equals_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(inf_equals_data), ab_parameters, inf_equals_body, inf_equals_filter, 0, NULL, 0);
    _x3C_x3D = Ov_Reference_new_symbol(inf_equals_data);

    Ov_UnknownData sup_equals_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(sup_equals_data), ab_parameters, sup_equals_body, sup_equals_filter, 0, NULL, 0);
    _x3E_x3D = Ov_Reference_new_symbol(sup_equals_data);

    Ov_UnknownData increment_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(increment_data), a_parameters, increment_body, increment_filter, 0, NULL, 0);
    _x2B_x2B = Ov_Reference_new_symbol(increment_data);

    Ov_UnknownData decrement_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(decrement_data), a_parameters, decrement_body, decrement_filter, 0, NULL, 0);
    _x2D_x2D = Ov_Reference_new_symbol(decrement_data);

    Ov_UnknownData add_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(add_data), ab_parameters, add_body, add_filter, 0, NULL, 0);
    _x3A_x2B_x3D = Ov_Reference_new_symbol(add_data);

    Ov_UnknownData remove_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(remove_data), ab_parameters, remove_body, remove_filter, 0, NULL, 0);
    _x3A_x2D_x3D = Ov_Reference_new_symbol(remove_data);

    Ov_UnknownData multiply_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(multiply_data), ab_parameters, multiply_body, multiply_filter, 0, NULL, 0);
    _x3A_x2A_x3D = Ov_Reference_new_symbol(multiply_data);

    Ov_UnknownData divide_data = {
        .vtable = &Ov_VirtualTable_Object,
        .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
    };
    Ov_Function_push(Ov_UnknownData_get_function(divide_data), ab_parameters, divide_body, divide_filter, 0, NULL, 0);
    _x3A_x2F_x3D = Ov_Reference_new_symbol(divide_data);
}
