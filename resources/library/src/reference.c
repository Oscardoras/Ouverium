#include <string.h>

#include "include.h"
#include "gc.h"


static Ov_GC_Reference Ov_Reference_copy_to(Ov_GC_Reference ref) {
    Ov_GC_Reference new_ref = ref;

    if (new_ref.type == TUPLE) {
        new_ref.tuple.references = Ov_GC_alloc_references(new_ref.tuple.size);
        size_t i;
        for (i = 0; i < new_ref.tuple.size; ++i)
            new_ref.tuple.references[i] = Ov_Reference_copy_to(ref.tuple.references[i]);
    }

    return new_ref;
}

Ov_Reference_Owned Ov_Reference_new_uninitialized() {
    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = DATA;
    reference->data.vtable = NULL;

    return (Ov_Reference_Owned)reference;
}

Ov_Reference_Owned Ov_Reference_new_data(Ov_UnknownData data) {
    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = DATA;
    reference->data = data;

    return (Ov_Reference_Owned)reference;
}

Ov_Reference_Owned Ov_Reference_new_symbol(Ov_UnknownData data) {
    Ov_UnknownData* d = Ov_GC_alloc_object(&Ov_VirtualTable_UnknownData);
    *d = data;

    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = SYMBOL;
    reference->symbol = d;

    return (Ov_Reference_Owned)reference;
}

Ov_Reference_Owned Ov_Reference_new_property(Ov_UnknownData parent, Ov_VirtualTable* vtable, unsigned int hash) {
    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = PROPERTY;
    reference->property.parent = parent;
    reference->property.vtable = vtable;
    reference->property.hash = hash;

    return (Ov_Reference_Owned)reference;
}

Ov_Reference_Owned Ov_Reference_new_array(Ov_UnknownData array, size_t i) {
    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = ARRAY;
    reference->array.array = array;
    reference->array.i = i;

    return (Ov_Reference_Owned)reference;
}

Ov_Reference_Owned Ov_Reference_new_tuple(Ov_Reference_Shared references[], size_t references_size, Ov_VirtualTable* vtable) {
    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = TUPLE;
    reference->tuple.references = Ov_GC_alloc_references(references_size);
    reference->tuple.size = references_size;
    reference->tuple.vtable = vtable;

    size_t i;
    for (i = 0; i < references_size; ++i)
        reference->tuple.references[i] = Ov_Reference_copy_to(*((Ov_GC_Reference*)references[i]));

    return (Ov_Reference_Owned)reference;
}

Ov_Reference_Owned Ov_Reference_new_string(const char* string, Ov_VirtualTable* vtable) {
    size_t size = strlen(string);

    Ov_GC_Reference* reference = Ov_GC_alloc_references(1);
    reference->type = TUPLE;
    reference->tuple.references = Ov_GC_alloc_references(size);
    reference->tuple.size = size;
    reference->tuple.vtable = vtable;

    size_t i;
    for (i = 0; i < size; i++) {
        Ov_Data data = { .c = string[i] };
        reference->tuple.references[i] = *((Ov_GC_Reference*)Ov_Reference_new_data(Ov_UnknownData_from_data(&Ov_VirtualTable_Char, data)));
    }

    return (Ov_Reference_Owned)reference;
}

Ov_UnknownData Ov_Reference_get(Ov_Reference_Shared r) {
    Ov_GC_Reference* reference = (Ov_GC_Reference*)r;

    switch (reference->type) {
    case DATA:
        return reference->data;
    case SYMBOL:
        return *reference->symbol;
    case PROPERTY:
        return Ov_UnknownData_from_ptr(reference->property.vtable, Ov_UnknownData_get_property(reference->property.parent, reference->property.hash));
    case ARRAY: {
        Ov_ArrayInfo array = Ov_UnknownData_get_array(reference->array.array);
        return  Ov_UnknownData_from_ptr(array.vtable, Ov_Array_get(array, reference->array.i));
    }
    case TUPLE: {
        Ov_UnknownData data = {
            .vtable = reference->tuple.vtable,
            .data.ptr = Ov_GC_alloc_object(reference->tuple.vtable)
        };

        Ov_ArrayInfo array = Ov_UnknownData_get_array(data);
        Ov_Array_set_size(array, reference->tuple.size);
        size_t i;
        for (i = 0; i < reference->tuple.size; ++i)
            Ov_UnknownData_set(array.vtable, Ov_Array_get(array, i), Ov_Reference_get((Ov_Reference_Shared)&reference->tuple.references[i]));

        return data;
    }
    default: {
        Ov_UnknownData data = {
            .vtable = NULL,
            .data.ptr = NULL
        };
        return data;
    }
    }
}

Ov_Reference_Owned Ov_Reference_get_element(Ov_Reference_Shared r, size_t i) {
    Ov_GC_Reference* reference = (Ov_GC_Reference*)r;

    if (reference->type == TUPLE) {
        return Ov_Reference_copy(Ov_Reference_share(&reference->tuple.references[i]));
    }
    else {
        Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(reference));
        return Ov_Reference_new_array(data, i);
    }
}

size_t Ov_Reference_get_size(Ov_Reference_Shared r) {
    Ov_GC_Reference* reference = (Ov_GC_Reference*)r;

    if (reference->type == TUPLE) {
        return reference->tuple.size;
    }
    else {
        Ov_UnknownData data = Ov_Reference_get(Ov_Reference_share(reference));
        Ov_ArrayInfo array = Ov_UnknownData_get_array(data);
        return array.array->size;
    }
}

Ov_Reference_Owned Ov_Reference_copy(Ov_Reference_Shared r) {
    Ov_GC_Reference* reference = (Ov_GC_Reference*)r;

    Ov_GC_Reference* new_reference = Ov_GC_alloc_references(1);
    *new_reference = Ov_Reference_copy_to(*reference);

    return (Ov_Reference_Owned)new_reference;
}

void Ov_Reference_free(Ov_Reference_Owned r) {
    Ov_GC_Reference* reference = (Ov_GC_Reference*)r;

    if (reference->type == TUPLE) {
        size_t i;
        for (i = 0; i < reference->tuple.size; ++i)
            Ov_Reference_free((Ov_Reference_Owned)(reference->tuple.references + i));
    }

    Ov_GC_free_reference(reference);
}
