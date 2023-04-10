#include <stdlib.h>

#include "gc.h"


__GC_Element* __GC_list;
__GC_Context __GC_contexts;

void __GC_NULL_iterator(void*) {}

void __GC_init(__GC_Iterator gc_global_context_iterator) {
    __GC_list = NULL;
    __GC_contexts.next = NULL;
    __GC_contexts.iterator = gc_global_context_iterator;
}

void* __GC_alloc_object(unsigned long size) {
    __GC_Element* ptr = malloc(sizeof(__GC_Element) + size);

    if (ptr == NULL) {
        __GC_collect();

        ptr = malloc(sizeof(__GC_Element) + size);
        if (ptr == NULL)
            return NULL;
    }

    ptr->next = __GC_list;
    ptr->iterated = false;
    __GC_list = ptr;
    return ptr + 1;
}

void __GC_collect(void) {
    for (__GC_Context* context = &__GC_contexts; context != NULL; context = context->next)
        context->iterator(context);

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

void __GC_end(void) {
    for (__GC_Element* ptr = __GC_list; ptr != NULL;) {
        __GC_Element* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}
