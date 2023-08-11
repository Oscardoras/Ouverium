#ifndef __GC_H__
#define __GC_H__

#include "include.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ROOTS_CAPACITY 128

    typedef struct __GC_Element {
        struct __GC_Element* next;
        __VirtualTable* vtable;
        bool iterated;
    } __GC_Element;

    typedef struct __GC_Reference {
        enum {
            NONE = 0,
            DATA,
            SYMBOL,
            PROPERTY,
            ARRAY,
            TUPLE
        } type;
        union {
            struct {
                size_t size;
                struct __GC_Reference* next;
            } none;

            __UnknownData data;
            __UnknownData* symbol;
            struct {
                __UnknownData parent;
                __VirtualTable* virtual_table;
                unsigned int hash;
            } property;
            struct {
                __UnknownData array;
                size_t i;
            } array;
            struct {
                struct __GC_Reference* references;
                size_t size;
            } tuple;
        };
    } __GC_Reference;

    typedef struct __GC_Roots {
        struct __GC_Roots* next;
        size_t capacity;
        __GC_Reference* free_list;
    } __GC_Roots;

    __GC_Roots* __GC_init_roots(size_t c);
    __GC_Reference* __GC_alloc_references(size_t n);
    void __GC_free_reference(__GC_Reference* reference);
    
    struct __FunctionCapture __GC_reference_to_capture(__GC_Reference* reference);
    __Reference_Owned __GC_capture_to_reference(struct __FunctionCapture capture);

#ifdef __cplusplus
}
#endif


#endif
