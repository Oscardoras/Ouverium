#ifndef INTERPRETER_REFERENCE_HPP_
#define INTERPRETER_REFERENCE_HPP_

#include "Object.hpp"


struct Reference {
    
    enum ReferenceType {
        //Tuple <= -3
        PointerCopy = -2,
        PointerReference = -1,
        //ArrayPointerReference >= 0
    };
    
    long type;

    union {
        Object* ptrCopy;
        Object* * ptrRef;
        Reference * tuple;
    };

    Reference();

    Reference(Reference const& reference);

    Reference(Object* const& pointer);

    Reference(Object** const& reference);

    Reference(Object** const& reference, unsigned long const& i);

    Reference(size_t const& tuple_size);

    ~Reference();

    bool isTuple() const;
    bool isPointerCopy() const;
    bool isReference() const;
    bool isPointerReference() const;
    bool isArrayReference() const;

    int getArrayIndex() const;
    size_t getTupleSize() const;
    Object** getArrayReference() const;

    Reference toSymbolReference() const;
    Object* toObject() const;

    Reference& operator=(Reference const& reference);

    void unuse();

};


#endif