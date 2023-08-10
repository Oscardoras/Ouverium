#include <stdlib.h>

#include "gc.h"

#define ROOTS_CAPACITY 128


__GC_Element* __GC_list;
__GC_Roots* __GC_roots;
__Reference __GC_moved_reference;

void __GC_init(void) {
    __GC_list = NULL;
}

void __GC_init_roots(void) {
    __GC_roots = malloc(sizeof(__GC_Roots) + sizeof(__Reference*) * ROOTS_CAPACITY);

    __GC_roots->next = NULL;
    __GC_roots->capacity = ROOTS_CAPACITY;
    __GC_roots->size = 0;
}

void __GC_add_reference(__Reference* reference) {
    if (__GC_roots->size >= __GC_roots->capacity) {
        size_t capacity = __GC_roots->capacity * 2;
        __GC_Roots* roots = malloc(sizeof(__GC_Roots) + sizeof(__Reference*) * capacity);

        roots->next = __GC_roots;
        roots->capacity = capacity;
        roots->size = 0;

        __GC_roots = roots;
    }

    __GC_roots->tab[__GC_roots->size++] = reference;
}

void __GC_move_reference(__Reference* reference) {
    __GC_moved_reference = *reference;
    __GC_remove_reference();
}

void __GC_remove_reference() {
    if (__GC_roots->size == 0) {
        __GC_Roots* old = __GC_roots;
        __GC_roots = __GC_roots->next;
        free(old);
    }

    --__GC_roots->size;
}

void __GC_end_roots(void) {
    while (__GC_roots != NULL) {
        __GC_Roots* old = __GC_roots;
        __GC_roots = __GC_roots->next;
        free(old);
    }
}

void* __GC_alloc_object(__VirtualTable* vtable) {
    __GC_Element* ptr = malloc(sizeof(__GC_Element) + vtable->size);

    if (ptr == NULL) {
        __GC_collect();

        ptr = malloc(sizeof(__GC_Element) + vtable->size);
        if (ptr == NULL)
            return NULL;
    }

    ptr->next = __GC_list;
    ptr->vtable = vtable;
    ptr->iterated = false;
    __GC_list = ptr;
    return ptr + 1;
}

void __GC_iterate(__GC_Iterator iterator, void* object) {
    __GC_Element* element = ((__GC_Element*)object) - 1;

    if (!element->iterated) {
        if (iterator != NULL)
            iterator(object);
        element->iterated = true;
    }
}

void __GC_Reference_iterator(__Reference* reference) {
    switch (reference->type) {
    case DATA:
        __GC_iterate(reference->data.virtual_table->gc_iterator, reference->data.data.ptr);
        break;
    case SYMBOL:
        __GC_iterate(reference->symbol->virtual_table->gc_iterator, reference->symbol->data.ptr);
        break;
    case PROPERTY:
        __GC_iterate(reference->property.parent.virtual_table->gc_iterator, reference->property.parent.data.ptr);
        break;
    case ARRAY:
        __GC_iterate(reference->array.array.virtual_table->gc_iterator, reference->array.array.data.ptr);
        break;
    case TUPLE: {
        size_t i;
        for (i = 0; i < reference->tuple.size; ++i)
            __GC_Reference_iterator(&reference->tuple.tab[i]);
        break;
    }
    default:
        break;
    }
}

void __GC_collect(void) {
    __GC_Roots* roots;
    for (roots = __GC_roots; roots != NULL; roots = roots->next) {
        size_t i;
        for (i = 0; i < roots->size; ++i)
            __GC_Reference_iterator(roots->tab[i]);
    }
    __GC_Reference_iterator(&__GC_moved_reference);

    __GC_Element** ptr;
    for (ptr = &__GC_list; *ptr != NULL;) {
        if (!(*ptr)->iterated) {
            __GC_Element* next = (*ptr)->next;
            if ((*ptr)->vtable->gc_destructor != NULL)
                (*ptr)->vtable->gc_destructor(*ptr);
            free(*ptr);
            *ptr = next;
        }
        else {
            (*ptr)->iterated = false;
            ptr = &(*ptr)->next;
        }
    }
}

void __GC_end(void) {
    __GC_Element* ptr;
    for (ptr = __GC_list; ptr != NULL;) {
        __GC_Element* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}
