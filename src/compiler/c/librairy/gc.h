#ifndef __GC_H__
#define __GC_H__


typedef struct GC_Element_t {
    void* next;
    bool white;
    enum {
        GC_ELEMENT_PRIMITIVE,
        GC_ELEMENT_Object,
        GC_ELEMENT_Array
    } type;
    union {
        char primitive;
        void* object;
        struct Data array;
    } data;
} GC_Element;


void GC_init();

void* GC_alloc_object(short size);

void GC_collect(void* *roots);

void GC_end();


#endif
