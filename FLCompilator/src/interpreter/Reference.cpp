#include "Context.hpp"
#include "Reference.hpp"


Reference::Reference() {
    type = PointerCopy;
    ptrCopy = nullptr;
}

Reference::Reference(Reference const& reference) {
    type = reference.type;
    
    if (type == PointerCopy || type >= 1) ptrCopy = reference.ptrCopy;
    else if (type == PointerReference) ptrRef = reference.ptrRef;
    else {
        auto n = getTupleSize();
        tuple = new Reference[n];
        for (long i = 0; i < n; i++)
            tuple[i] = reference.tuple[i];
    }
}

Reference::Reference(Object* const& object) {
    type = PointerCopy;
    ptrCopy = object;
}

Reference::Reference(Object** const& reference) {
    type = PointerReference;
    ptrRef = reference;
}

Reference::Reference(Object* const& reference, unsigned long const& i) {
    type = i;
    ptrCopy = reference;
}

Reference::Reference(size_t const& tuple_size) {
    type = -tuple_size;
    tuple = new Reference[tuple_size];
}

Reference::~Reference() {
    if (isTuple())
        delete[] tuple;
}

bool Reference::isTuple() const {
    return type <= -2;
}

bool Reference::isPointerCopy() const {
    return type == PointerCopy;
}

bool Reference::isPointerReference() const {
    return type == PointerReference;
}

bool Reference::isArrayReference() const {
    return type > 0;
}

long Reference::getArrayIndex() const {
    return type;
}

long Reference::getTupleSize() const {
    return -type;
}

Object** Reference::getArrayReference() const {
    return &ptrCopy->data.a[type].o;
}

Reference Reference::toSymbolReference() const {
    if (isTuple()) {
        auto n = getTupleSize();
        auto reference = Reference(new Object((size_t) n));
        for (long i = 0; i < n; i++)
            reference.ptrCopy->data.a[i+1].o = tuple[i].toObject();
        return reference;
    } else return *this;
}

Object* Reference::toObject() const {
    if (type == PointerCopy) return ptrCopy;
    else if (type == PointerReference) return *ptrRef;
    else if (type > 0) return *getArrayReference();
    else {
        auto n = getTupleSize();
        auto object = new Object((size_t) n);
        for (long i = 0; i < n; i++)
            object->data.a[i+1].o = tuple[i].toObject();
        return object;
    }
}

Reference& Reference::operator=(Reference const& reference) {
    if (isTuple())
        delete[] tuple;

    type = reference.type;
    
    if (type == PointerCopy || type >= 1) ptrCopy = reference.ptrCopy;
    else if (type == PointerReference) ptrRef = reference.ptrRef;
    else {
        auto n = getTupleSize();
        tuple = new Reference[n];
        for (long i = 0; i < n; i++)
            tuple[i] = reference.tuple[i];
    }
    
    return *this;
}