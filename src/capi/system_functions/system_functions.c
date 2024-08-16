#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <ouverium/include.h>

#include "../gc.h"


void Ov_init_functions_base();
void Ov_init_functions_dll();
void Ov_init_functions_math();

void Ov_init_functions() {
    Ov_init_functions_base();
    Ov_init_functions_dll();
    Ov_init_functions_math();
}

Ov_UnknownData Ov_get_object(Ov_Reference_Shared reference) {
    Ov_GC_Reference* ref = (Ov_GC_Reference*) reference;

    switch (ref->type) {
        case SYMBOL: {
            if (ref->symbol->vtable == NULL) {
                Ov_UnknownData data = {
                    .vtable = &Ov_VirtualTable_Object,
                    .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
                };

                *ref->symbol = data;
            }

            return *ref->symbol;
        }
        case PROPERTY: {
            Ov_PropertyInfo property = Ov_UnknownData_get_property(ref->property.parent, ref->property.hash);

            if (property.vtable == NULL) {
                Ov_UnknownData data = {
                    .vtable = &Ov_VirtualTable_Object,
                    .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
                };

                Ov_UnknownData_set(property.vtable, property.ptr, data);
            }

            return Ov_UnknownData_from_ptr(property.vtable, property.ptr);
        }
        case ARRAY: {
            Ov_ArrayInfo array = Ov_UnknownData_get_array(ref->array.array);

            if (array.vtable == NULL) {
                Ov_UnknownData data = {
                    .vtable = &Ov_VirtualTable_Object,
                    .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object)
                };

                Ov_UnknownData_set(array.vtable, Ov_Array_get(array, ref->array.i), data);
            }

            return Ov_UnknownData_from_ptr(array.vtable, Ov_Array_get(array, ref->array.i));
        }
        default:
            Ov_UnknownData data = {
                .vtable = NULL,
                .data.ptr = NULL
            };
            return data;
    }
}
