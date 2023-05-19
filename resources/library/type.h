#ifndef __TYPE_H__
#define __TYPE_H__

#include "function.h"


#ifdef __cplusplus

template<typename... Components>
class Type: public Components {

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
    static constexpr void add_vtable(void* *vtable) {}
    template<typename C, typename... Cs>
    static constexpr void add_vtable(void* *vtable) {
        ((char*) vtable)[offset<C>] = (char*) static_cast<C*>(&object) - (char*) &object;
        create_vtable<Cs>();
    }

    template<typename... Cs>
    static constexpr bool create_vtable(void* *vtable) {
        vtable[0] = sizeof(Type<Components>);
        vtable[1] = __GC_iterator;
        add_vtable<Cs>();
        return true;
    }

protected:

    static void __GC_iterator(Type<Components>* object) {
        object.iterate();
    }

public:

    inline static __VirtualTable const vtable;

    static_assert(create_vtable(vtable));

};

typedef Type<int> Integer;

#endif


#endif
