#include <stdlib.h>

#include "gc.h"


__GC_Element* __GC_list;
_Thread_local __GC_Context* __GC_contexts = NULL;

void __GC_NULL_iterator(void*) {}

void __GC_init() {
    __GC_list = NULL;
}

void* __GC_alloc_object(size_t size) {
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
    for (__GC_Context* context = __GC_contexts; context != NULL; context = context->parent)
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
