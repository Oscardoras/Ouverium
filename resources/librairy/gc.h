#ifndef __GC_H__
#define __GC_H__

#include <stdbool.h>


typedef union __Data {
    void* ptr;
    long i;
    double d;
    char c;
    bool b;
} __Data;


typedef void (*__GC_Iterator)(void*);

typedef struct __GC_Context {
    struct __GC_Context* next;
    __GC_Iterator iterator;
} __GC_Context;

typedef struct __GC_Element {
    struct __GC_Element* next;
    bool iterated;
} __GC_Element;

void GC_init(__GC_Iterator gc_global_context_iterator);
__GC_Element* GC_alloc_object(unsigned long size);
void GC_collect(void);
void GC_end(void);


typedef __UnknownData (*__Array_Iterator)(void* array, unsigned long i);

typedef struct __VirtualTable {
    __GC_Iterator gc_iterator;
    __Array_Iterator array_iterator;
} __VirtualTable;

typedef struct __UnknownData {
    __VirtualTable* virtual_table;
    __Data data;
} __UnknownData;


typedef struct __Reference {
    enum {
        DATA,
        REFERENCE,
        PROPERTY,
        ARRAY,
    } type;
    union {
        __UnknownData data;
        __UnknownData* reference;
        struct {
            __UnknownData object;
            __UnknownData property;
        } property;
        struct {
            __UnknownData array;
            unsigned long i;
        } array;
    };
} __Reference;

__UnknownData GC_Reference_get(__Reference reference);


typedef bool (*__FunctionFilter)(__GC_Context* context, __Reference args);
typedef __Reference (*__FunctionBody)(__GC_Context* context, __Reference args);

typedef struct __Function {
    __Function* next;
    __FunctionFilter filter;
    __FunctionBody body;
    unsigned short references_size;
    __Reference* references;
} __Function;

__Reference __GC_eval_function(__GC_Context* context, __Function* function, __Reference args);


#endif
