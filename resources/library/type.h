#ifndef __TYPE_H__
#define __TYPE_H__

#include "function.h"


#ifdef __cplusplus

template<typename ArrayType, bool function, typename... Components>
class Type: public std::conditional<std::is_same<ArrayType, void>::value, void, ArrayType::vtable>, public std::conditional<true, __Function, void>, public Components... {

private:

    template<>
    void iterate() {}
    template<typename C, typename... Cs>
    void iterate() {
        __GC_iterate(static_cast<C*>(this));
        iterate<Cs>();
    }

    constexpr Type<Components> object;

    template<>
    static constexpr void add_vtable(__VirtualTable* vtable) {}
    template<typename C, typename... Cs>
    static constexpr void add_vtable(__VirtualTable* vtable) {
        vtable->tab[index<C>].offset = (size_t) ((char*) static_cast<C*>(&object)) - ((char*) &object);
        add_vtable<Cs>();
    }
    static constexpr bool create_vtable(__VirtualTable* vtable) {
        vtable->size = sizeof(Type<Components>);
        vtable->gc_iterator = __GC_iterator;
        vtable->array.vtable = ArrayType::vtable;
        vtable->array.offset = 0;
        vtable->function_offset
        add_vtable<Components>();
        return true;
    }

protected:

    static void __GC_iterator(Type<Components>* object) {
        object.iterate();
    }

public:

    inline static __VirtualTable const vtable;

    static_assert(create_vtable<Components>(vtable));

};

typedef Type<void, false, int> Integer;

void tes() {
    Integer i;
}

#endif


#endif
