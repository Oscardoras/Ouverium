#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "gc.h"


GC_Element* GC_list;

void GC_init() {
    GC_list = NULL;
}

void* GC_alloc_object(unsigned long size, void (*iterator)(void*)) {
    GC_Element* ptr = malloc(sizeof(GC_Element) + size);

    if (ptr == NULL) {

        ptr = malloc(sizeof(GC_Element) + size);
        if (ptr == NULL)
            return NULL;
    }

    ptr->next = GC_list;
    ptr->iterated = false;
    ptr->iterator = iterator;
    GC_list = ptr;
    return ptr + 1;
}

void GC_iterate(GC_Element* element) {
    element->iterated = true;
    element->iterator(element + 1);
}

void GC_collect(GC_Element* *roots) {
    for (; *roots != NULL; roots++) {
        GC_iterate(*roots);
    }

    for (GC_Element** ptr = &GC_list; *ptr != NULL;) {
        if (!(*ptr)->iterated) {
            GC_Element* next = (*ptr)->next;
            free(*ptr);
            *ptr = next;
        } else {
            (*ptr)->iterated = false;
            ptr = &(*ptr)->next;
        }
    }
}

void GC_end() {
    for (GC_Element* ptr = GC_list; ptr != NULL;) {
        GC_Element* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}

GC_FunctionBody GC_eval_function(GC_Function* function, void* args) {
    for (GC_Function* ptr = function; ptr != NULL; ptr = ptr->next) {
        if (function->filter == NULL || function->filter(function+1, args)) {
            function->body(function+1, args);
            break;
        }
    }
}

void GC_Array_iterator(void* element) {
    GC_Array* array = (GC_Array*) element;
    for (unsigned long i = 0; i < array->size; i++) {
        GC_iterate(array->tab[i]);
    }
}

void GC_Array_set_capacity(GC_Array* array, unsigned long capacity) {
    array->tab = realloc(array->tab, capacity);
    array->capacity = capacity;
}
