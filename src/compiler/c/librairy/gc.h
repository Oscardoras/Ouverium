#ifndef __GC_H__
#define __GC_H__


typedef struct GC_Element {
    void* next;
    bool iterated;
    void (*iterator)(void*);
} GC_Element;

void GC_init();
void* GC_alloc_object(unsigned long size, void (*iterator)(void*));
void GC_iterate(GC_Element*);
void GC_collect(GC_Element* *roots);
void GC_end();


typedef void* (*GC_FunctionBody)(void* context, void* args);

typedef struct GC_Function {
    GC_Function* next;
    bool (*filter)(void* context, void* args);
    GC_FunctionBody body;
} GC_Function;

GC_FunctionBody GC_eval_function(GC_Function* function, void* args);


typedef struct GC_Array {
    unsigned long size;
    unsigned long capacity;
    void* *tab;
} GC_Array;

void GC_Array_iterator(void*);


#endif
