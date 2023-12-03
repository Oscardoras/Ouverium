#include <string.h>

#include "include.h"
#include "gc.h"


__Reference_Owned __Reference_new_data(__UnknownData data) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = DATA;
    reference->data = data;

    return (__Reference_Owned)reference;
}

__Reference_Owned __Reference_new_symbol(__UnknownData data) {
    __UnknownData* d = __GC_alloc_object(&__VirtualTable_UnknownData);
    *d = data;

    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = SYMBOL;
    reference->symbol = d;

    return (__Reference_Owned)reference;
}

__Reference_Owned __Reference_new_property(__UnknownData parent, __VirtualTable* virtual_table, unsigned int hash) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = PROPERTY;
    reference->property.parent = parent;
    reference->property.virtual_table = virtual_table;
    reference->property.hash = hash;

    return (__Reference_Owned)reference;
}

__Reference_Owned __Reference_new_array(__UnknownData array, size_t i) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = PROPERTY;
    reference->array.array = array;
    reference->array.i = i;

    return (__Reference_Owned)reference;
}

__Reference_Owned __Reference_new_tuple(__Reference_Shared references[], size_t references_size) {
    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = TUPLE;
    reference->tuple.references = __GC_alloc_references(references_size);
    reference->tuple.size = references_size;

    size_t i;
    for (i = 0; i < references_size; i++)
        reference->tuple.references[i] = *((__GC_Reference*)references[i]);

    return (__Reference_Owned)reference;
}

__Reference_Owned __Reference_new_string(const char *string) {
    size_t size = strlen(string);

    __GC_Reference* reference = __GC_alloc_references(1);
    reference->type = TUPLE;
    reference->tuple.references = __GC_alloc_references(size);
    reference->tuple.size = size;

    size_t i;
    for (i = 0; i < size; i++) {
        __Data data = { .c = string[i] };
        reference->tuple.references[i] = *((__GC_Reference*) __Reference_new_data(__UnknownData_from_data(&__VirtualTable_Char, data)));
    }

    return (__Reference_Owned)reference;
}

__UnknownData __Reference_get(__Reference_Shared r) {
    __GC_Reference* reference = (__GC_Reference*)r;

    switch (reference->type) {
    case DATA:
        return reference->data;
    case SYMBOL:
        return *reference->symbol;
    case PROPERTY:
        return __UnknownData_from_ptr(reference->property.virtual_table, __UnknownData_get_property(reference->property.parent, reference->property.hash));
    case ARRAY: {
        __ArrayInfo array = __UnknownData_get_array(reference->array.array);
        return  __UnknownData_from_ptr(array.vtable, __Array_get(array, reference->array.i));
    }
    case TUPLE: {
        __ArrayInfo array = {
            .vtable = &__VirtualTable_UnknownData,
            .array = __GC_alloc_object(&__VirtualTable_Array)
        };
        *array.array = __Array_new(&__VirtualTable_UnknownData, reference->tuple.size);
        array.array->size = reference->tuple.size;

        size_t i;
        for (i = 0; i < reference->tuple.size; ++i)
            *((__UnknownData*)__Array_get(array, i)) = __Reference_get((__Reference_Shared)&reference->tuple.references[i]);

        __UnknownData data = {
            .virtual_table = &__VirtualTable_Array,
            .data.ptr = array.array
        };
        return data;
    }
    default: {
        __UnknownData data = {
            .virtual_table = NULL,
            .data.ptr = NULL
        };
        return data;
    }
    }
}

__Reference_Owned __Reference_get_element(__Reference_Shared r, size_t i) {
    __GC_Reference* reference = (__GC_Reference*)r;

    if (reference->type == TUPLE) {
        return __Reference_copy(__Reference_share(&reference->tuple.references[i]));
    }
    else {
        __UnknownData data = __Reference_get(__Reference_share(reference));
        return __Reference_new_array(data, i);
    }
}

size_t __Reference_get_size(__Reference_Shared r) {
    __GC_Reference* reference = (__GC_Reference*)r;

    if (reference->type == TUPLE) {
        return reference->tuple.size;
    }
    else {
        __UnknownData data = __Reference_get(__Reference_share(reference));
        __ArrayInfo array = __UnknownData_get_array(data);
        return array.array->size;
    }
}

__Reference_Owned __Reference_copy(__Reference_Shared r) {
    __GC_Reference* reference = (__GC_Reference*)r;

    __GC_Reference* new_reference = __GC_alloc_references(1);
    *new_reference = *reference;

    if (new_reference->type == TUPLE) {
        new_reference->tuple.references = __GC_alloc_references(new_reference->tuple.size);
        memcpy(new_reference->tuple.references, reference->tuple.references, sizeof(__GC_Reference) * new_reference->tuple.size);
    }

    return (__Reference_Owned)new_reference;
}

void __Reference_free(__Reference_Owned r) {
    __GC_Reference* reference = (__GC_Reference*)r;

    if (reference->type == TUPLE) {
        size_t i;
        for (i = 0; i < reference->tuple.size; i++)
            __Reference_free((__Reference_Owned)reference->tuple.references + i);
    }

    __GC_free_reference(reference);
}
