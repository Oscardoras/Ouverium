#ifndef __GC_H__
#define __GC_H__


typedef struct GC_Element_t {
    void* next;
    bool iterated;
    void (*iterator)(GC_Element*);
} GC_Element;


void GC_init();

void* GC_alloc_object(unsigned long size, void (*iterator)(GC_Element*));

void GC_collect(GC_Element* *roots);

void GC_end();


#endif
