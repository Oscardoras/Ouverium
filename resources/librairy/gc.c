#include <stdlib.h>

#include "gc.h"


__GC_Element* __GC_list;
__GC_Context __GC_contexts;

void __GC_init(__GC_Iterator gc_global_context_iterator) {
    __GC_list = NULL;
    __GC_contexts.next = NULL;
    __GC_contexts.iterator = gc_global_context_iterator;
}

__GC_Element* GC_alloc_object(unsigned long size) {
    __GC_Element* ptr = malloc(sizeof(__GC_Element) + size);

    if (ptr == NULL) {
        GC_collect();

        ptr = malloc(sizeof(__GC_Element) + size);
        if (ptr == NULL)
            return NULL;
    }

    ptr->next = __GC_list;
    ptr->iterated = false;
    __GC_list = ptr;
    return ptr;
}

void GC_collect() {
    for (__GC_Context* context; context != NULL; context = context->next)
        context->iterator();

    for (__GC_Element** ptr = &__GC_list; *ptr != NULL;) {
        if (!(*ptr)->iterated) {
            __GC_Element* next = (*ptr)->next;
            free(*ptr);
            *ptr = next;
        } else {
            (*ptr)->iterated = false;
            ptr = &(*ptr)->next;
        }
    }
}

void GC_end() {
    for (__GC_Element* ptr = __GC_list; ptr != NULL;) {
        __GC_Element* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}

__UnknownData GC_Reference_get(__Reference reference) {
    switch (reference.type) {
        case DATA:
            return reference.data;

        case REFERENCE:
            return *reference.reference;
        
        case PROPERTY:
            return reference.property.property;
        
        case ARRAY:
            return reference.array.array.virtual_table->array_iterator(reference.array.array.data.ptr, reference.array.i);
        default:
            break;
    }
}

__Reference __GC_eval_function(__GC_Context* context, __Function* function, __Reference args) {
    for (__Function* ptr = function; ptr != NULL; ptr = ptr->next) {
        if (function->filter == NULL || function->filter(context, args)) {
            function->body(context, args);
            break;
        }
    }
}
