#include <stdlib.h>

#include "gc.h"


__GC_Element* __GC_list;
__GC_Roots* __GC_roots;

void __GC_NULL_iterator(void*) {}

void __GC_init(void) {
    __GC_list = NULL;
    __GC_roots = __GC_init_roots(ROOTS_CAPACITY);
}

__GC_Roots* __GC_init_roots(size_t c) {
    __GC_Roots* roots = malloc(sizeof(__GC_Roots) + sizeof(__GC_Reference) * c);

    roots->next = NULL;
    roots->capacity = c;
    roots->free_list = NULL;

    __GC_Reference* tab = (__GC_Reference*) roots+1;
    tab[0].type = NONE;
    tab[0].none.size = c;
    tab[0].none.next = NULL;

    return roots;
}

__GC_Reference *__GC_alloc_references(size_t n) {
    __GC_Roots* roots = __GC_roots;
    __GC_Reference** reference_ptr = &roots->free_list;

    while (!(*reference_ptr)->type == NONE && (*reference_ptr)->none.size >= n) {
        if ((*reference_ptr)->none.next != NULL) {
            reference_ptr = &(*reference_ptr)->none.next;
        } else {
            if (roots->next == NULL)
                roots->next = __GC_init_roots(roots->capacity * 2 > n ? roots->capacity * 2 : n);

            roots = roots->next;
            reference_ptr = &roots->free_list;
        }
    }

    if ((*reference_ptr)->none.size > n) {
        (*reference_ptr+n)->none.size = (*reference_ptr)->none.size - n;
        (*reference_ptr+n)->none.next = (*reference_ptr)->none.next;

        *reference_ptr = (*reference_ptr+n)->none.next;
    } else {
        *reference_ptr = (*reference_ptr)->none.next;
    }
    return *reference_ptr;
}

void __GC_free_reference(__GC_Reference* reference) {
    struct __GC_Roots* roots = __GC_roots;
    while (!((__GC_Reference*) roots < reference && reference < ((__GC_Reference*) roots+1) + roots->capacity))
        roots = roots->next;

    reference->type = NONE;
    reference->none.size = 1;
    reference->none.next = roots->free_list;
    roots->free_list = reference;
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

void __GC_iterate(__GC_Iterator iterator, void* object) {
    __GC_Element* element = ((__GC_Element*) object) - 1;

    if (!element->iterated) {
        iterator(object);
        element->iterated = true;
    }
}

void __GC_Reference_iterator(__GC_Reference* reference) {
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
                __GC_Reference_iterator(&reference->tuple.references[i]);
            break;
        }
        default:
            break;
    }
}

void __GC_collect(void) {
    __GC_Roots** roots_ptr;
    for (roots_ptr = &__GC_roots; *roots_ptr != NULL;) {
        __GC_Reference** reference_ptr = &(*roots_ptr)->free_list;
        __GC_Reference* last_free = NULL;

        size_t i;
        for (i = 0; i < (*roots_ptr)->capacity;) {
            __GC_Reference* reference = ((__GC_Reference*) (*roots_ptr)+1) + i;
            if (reference->type == NONE) {
                if (last_free != NULL) {
                    last_free->none.size++;
                    ++i;
                } else {
                    *reference_ptr = reference;
                    reference_ptr = &(*reference_ptr)->none.next;
                    last_free = reference;
                    i += reference->none.size;
                }
            } else {
                __GC_Reference_iterator(reference);
                last_free = NULL;
                ++i;
            }
        }

        if ((*roots_ptr)->free_list == NULL) {
            __GC_Roots* tmp = *roots_ptr;
            *roots_ptr = (*roots_ptr)->next;
            free(tmp);
        } else {
            roots_ptr = &(*roots_ptr)->next;
        }
    }

    __GC_Element** ptr;
    for (ptr = &__GC_list; *ptr != NULL;) {
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
    __GC_Roots* roots;
    for (roots = __GC_roots; roots != NULL;) {
        __GC_Roots* next = roots->next;
        free(roots);
        roots = next;
    }

    __GC_Element* ptr;
    for (ptr = __GC_list; ptr != NULL;) {
        __GC_Element* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}
