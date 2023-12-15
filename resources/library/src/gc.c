#include <assert.h>
#include <stdlib.h>

#include "gc.h"


Ov_GC_Element* Ov_GC_list;
Ov_GC_Roots* Ov_GC_roots;

void Ov_init(void) {
    Ov_GC_list = NULL;
    Ov_GC_roots = Ov_GC_init_roots(ROOTS_CAPACITY);

    Ov_init_functions();
}

Ov_GC_Roots* Ov_GC_init_roots(size_t c) {
    Ov_GC_Roots* roots = malloc(sizeof(Ov_GC_Roots) + sizeof(Ov_GC_Reference) * c);

    roots->next = NULL;
    roots->capacity = c;
    roots->free_list = (Ov_GC_Reference*)roots + 1;

    Ov_GC_Reference* tab = (Ov_GC_Reference*)roots + 1;
    tab[0].type = NONE;
    tab[0].none.size = c;
    tab[0].none.next = NULL;

    return roots;
}

Ov_GC_Reference* Ov_GC_alloc_references(size_t n) {
    Ov_GC_Roots* roots = Ov_GC_roots;
    Ov_GC_Reference** reference_ptr = &roots->free_list;

    assert((*reference_ptr)->type == NONE);
    while (!(*reference_ptr)->none.size >= n) {
        if ((*reference_ptr)->none.next != NULL) {
            reference_ptr = &(*reference_ptr)->none.next;
            assert((*reference_ptr)->type == NONE);
        }
        else {
            if (roots->next == NULL)
                roots->next = Ov_GC_init_roots(roots->capacity * 2 > n ? roots->capacity * 2 : n);

            roots = roots->next;
            reference_ptr = &roots->free_list;
        }
    }

    Ov_GC_Reference* ref = *reference_ptr;
    if ((*reference_ptr)->none.size > n) {
        (*reference_ptr + n)->type = NONE;
        (*reference_ptr + n)->none.size = (*reference_ptr)->none.size - n;
        (*reference_ptr + n)->none.next = (*reference_ptr)->none.next;

        *reference_ptr = *reference_ptr + n;
    }
    else {
        *reference_ptr = (*reference_ptr)->none.next;
    }
    return ref;
}

void Ov_GC_free_reference(Ov_GC_Reference* reference) {
    assert(reference->type != NONE);

    struct Ov_GC_Roots* roots = Ov_GC_roots;
    while (!((Ov_GC_Reference*)roots < reference && reference < ((Ov_GC_Reference*)roots + 1) + roots->capacity))
        roots = roots->next;

    reference->type = NONE;
    reference->none.size = 1;
    reference->none.next = roots->free_list;
    roots->free_list = reference;
}

void* Ov_GC_alloc_object(Ov_VirtualTable* vtable) {
    Ov_GC_Element* ptr = malloc(sizeof(Ov_GC_Element) + vtable->size);

    if (ptr == NULL) {
        Ov_GC_collect();

        ptr = malloc(sizeof(Ov_GC_Element) + vtable->size);
        if (ptr == NULL)
            return NULL;
    }

    ptr->next = Ov_GC_list;
    ptr->vtable = vtable;
    ptr->iterated = false;
    Ov_GC_list = ptr;

    Ov_UnknownData data = {
        .vtable = vtable,
        .data.ptr = ptr + 1
    };
    if (vtable->array.vtable) {
        Ov_ArrayInfo array = Ov_UnknownData_get_array(data);
        array.array->capacity = 0;
        array.array->size = 0;
        array.array->tab = NULL;
    }
    if (vtable->function.offset != -1) {
        *Ov_UnknownData_get_function(data) = NULL;
    }

    return data.data.ptr;
}

void Ov_GC_iterate(Ov_GC_Iterator iterator, void* object) {
    Ov_GC_Element* element = ((Ov_GC_Element*)object) - 1;

    if (!element->iterated) {
        if (iterator != NULL)
            iterator(object);
        element->iterated = true;
    }
}

void Ov_GC_Reference_iterator(Ov_GC_Reference* reference) {
    switch (reference->type) {
    case DATA:
        Ov_GC_iterate(reference->data.vtable->gc_iterator, reference->data.data.ptr);
        break;
    case SYMBOL:
        Ov_GC_iterate(reference->symbol->vtable->gc_iterator, reference->symbol->data.ptr);
        break;
    case PROPERTY:
        Ov_GC_iterate(reference->property.parent.vtable->gc_iterator, reference->property.parent.data.ptr);
        break;
    case ARRAY:
        Ov_GC_iterate(reference->array.array.vtable->gc_iterator, reference->array.array.data.ptr);
        break;
    case TUPLE: {
        size_t i;
        for (i = 0; i < reference->tuple.size; ++i)
            Ov_GC_Reference_iterator(&reference->tuple.references[i]);
        break;
    }
    default:
        break;
    }
}

void Ov_GC_collect(void) {
    Ov_GC_Roots** roots_ptr;
    for (roots_ptr = &Ov_GC_roots; *roots_ptr != NULL;) {
        Ov_GC_Reference** reference_ptr = &(*roots_ptr)->free_list;
        Ov_GC_Reference* last_free = NULL;

        size_t i;
        for (i = 0; i < (*roots_ptr)->capacity;) {
            Ov_GC_Reference* reference = ((Ov_GC_Reference*)(*roots_ptr) + 1) + i;
            if (reference->type == NONE) {
                if (last_free != NULL) {
                    last_free->none.size++;
                    ++i;
                }
                else {
                    *reference_ptr = reference;
                    reference_ptr = &(*reference_ptr)->none.next;
                    last_free = reference;
                    i += reference->none.size;
                }
            }
            else {
                Ov_GC_Reference_iterator(reference);
                last_free = NULL;
                ++i;
            }
        }

        if ((*roots_ptr)->free_list == NULL) {
            Ov_GC_Roots* tmp = *roots_ptr;
            *roots_ptr = (*roots_ptr)->next;
            free(tmp);
        }
        else {
            roots_ptr = &(*roots_ptr)->next;
        }
    }

    Ov_GC_Element** ptr;
    for (ptr = &Ov_GC_list; *ptr != NULL;) {
        if (!(*ptr)->iterated) {
            Ov_GC_Element* next = (*ptr)->next;
            free(*ptr);
            *ptr = next;
        }
        else {
            (*ptr)->iterated = false;
            ptr = &(*ptr)->next;
        }
    }
}

void Ov_end(void) {
    Ov_GC_Roots* roots;
    for (roots = Ov_GC_roots; roots != NULL;) {
        Ov_GC_Roots* next = roots->next;
        free(roots);
        roots = next;
    }

    Ov_GC_Element* ptr;
    for (ptr = Ov_GC_list; ptr != NULL;) {
        Ov_GC_Element* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}
