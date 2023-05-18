#ifndef __TYPE_H__
#define __TYPE_H__

#include "function.h"


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

template<typename T, size_t offset = T::offset>
class Component: public T {
    static
};

template<typename T>
class Type: public T {

protected:

    static void __GC_iterator(Type* object) {
        object.iterate();
    }

public:

    inline static __VirtualTable const vtable = {
        .gc_iterator = __GC_iterator;
        .get_array = __VirtualTable_NULL_get_array;
        .size = sizeof(Type<ArrayType>)
    };

};

template<typename T, typename ArrayType>
class Type: public T {

protected:

    __Array array;

    static void __GC_iterator(Type* object) {
        object.iterate();
    }

public:

    inline static __VirtualTable const vtable = {
        .gc_iterator = __GC_iterator;
        .get_array = __VirtualTable_NULL_get_array;
        .size = sizeof(Type<ArrayType>)
    };

};

typedef Type<int>;

#endif


#endif
