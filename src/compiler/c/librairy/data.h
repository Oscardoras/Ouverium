#ifndef __DATA_H__
#define __DATA_H__


typedef struct {

    enum {
        GC_ELEMENT_PRIMITIVE,
        GC_ELEMENT_OBJECT,
        GC_ELEMENT_ARRAY
    } type;

    union {
        char primitive;
        void* object;
    } data;

} Object;


#endif
