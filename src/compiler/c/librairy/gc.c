#include <stdbool.h>
#include <stdlib.h>

#include "gc.h"


GC_Element* GC_list;

void GC_init() {
    GC_list = NULL;
}

void* GC_alloc_object(unsigned long size) {
    GC_Element* ptr = malloc(size + sizeof(GC_Element));
    ptr->next = GC_list;
    ptr->white = false;
    GC_list = ptr;
    return &ptr->data;
}

void GC_collect(void* *roots) {
    for (; *roots != NULL; roots++) {

    }
}

void GC_end() {
    for (GC_Element* ptr = GC_list; ptr != NULL;) {
        GC_Element* tmp = ptr->next;
        free(ptr);
        ptr = tmp;
    }
}
