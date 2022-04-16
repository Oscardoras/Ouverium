#ifndef INTERPRETER_REFERENCE_HPP_
#define INTERPRETER_REFERENCE_HPP_

#include "Object.hpp"

class Context;


struct Reference {
    
    enum ReferenceType {
        SymbolReference = -3,
        PropertyReference = -2,
        ArrayReference = -1,
        Pointer = 0
        //Tuple > 0
    };
    
    long type;

    union {
        Object* * symbolReference;
        struct {
            Object* parent;
            Object* * reference;
        } propertyRefrence;
        struct {
            Object* array;
            unsigned long i;
        } arrayReference;
        Object* pointer;
        Reference * tuple;
    };

    Reference();

    Reference(Reference const& reference);

    Reference(Object** const& reference);

    Reference(Object* const& parent, Object** const& reference);

    Reference(Object* const& array, unsigned long const& i);

    Reference(Object* const& pointer);

    Reference(size_t const& tuple_size);

    ~Reference();

    bool isReference() const;
    Object* & getReference() const;

    Object* toObject(Context & context) const;

    Reference& operator=(Reference const& reference);

};


#endif