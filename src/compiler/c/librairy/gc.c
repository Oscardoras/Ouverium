#include <stdbool.h>
#include <stdlib.h>

#include "gc.h"


GC_Element* GC_list;

void GC_init() {
    GC_list = NULL;
}

void* GC_alloc_object(unsigned long size, void (*iterator)(GC_Element*)) {
    GC_Element* ptr = malloc(sizeof(GC_Element) + size);
    ptr->next = GC_list;
    ptr->iterated = false;
    ptr->iterator = iterator;
    GC_list = ptr;
    return ptr + 1;
}

void GC_collect(GC_Element* *roots) {
    for (; *roots != NULL; roots++) {
        (*roots)->iterated = true;
        (*roots)->iterator(*roots);
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
