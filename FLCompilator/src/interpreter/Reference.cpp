#include "Reference.hpp"


Reference::Reference() {
    type = PointerCopy;
    ptrCopy = nullptr;
}

Reference::Reference(Reference const& reference) {
    type = reference.type;
    
    if (type == PointerCopy) ptrCopy = reference.ptrCopy;
    else if (type >= PointerReference) ptrRef = reference.ptrRef;
    else {
        auto n = getTupleSize();
        tuple = new Reference[n];
        for (unsigned long i = 0; i < n; i++)
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

Reference::Reference(Object** const& reference, unsigned long const& i) {
    type = i;
    ptrRef = reference;
}

Reference::Reference(size_t const& tuple_size) {
    type = -3-tuple_size;
    tuple = new Reference[tuple_size];
}

Reference::~Reference() {
    if (isTuple())
        delete[] tuple;
}

bool Reference::isTuple() const {
    return type <= -3;
}

bool Reference::isPointerCopy() const {
    return type == PointerCopy;
}

bool Reference::isReference() const {
    return type >= PointerReference;
}

bool Reference::isPointerReference() const {
    return type == PointerReference;
}

bool Reference::isArrayReference() const {
    return type >= 0;
}

int Reference::getArrayIndex() const {
    return type;
}

size_t Reference::getTupleSize() const {
    return -3-type;
}

Object** Reference::getArrayReference() const {
    return &(*ptrRef)->data.a[type+1].o;
}

Reference Reference::toSymbolReference() const {
    if (isTuple()) {
        auto n = getTupleSize();
        auto reference = Reference(new Object(n));
        for (unsigned long i = 0; i < n; i++)
            reference.ptrCopy->data.a[i+1].o = tuple[i].toObject();
        return reference;
    } else return *this;
}

Object* Reference::toObject() const {
    if (type == PointerCopy) return ptrCopy;
    else if (type == PointerReference) return *ptrRef;
    else if (type > PointerReference) return *getArrayReference();
    else {
        auto n = getTupleSize();
        auto object = new Object(n);
        for (unsigned long i = 0; i < n; i++)
            object->data.a[i+1].o = tuple[i].toObject();
        return object;
    }
}

Reference& Reference::operator=(Reference const& reference) {
    if (isTuple())
        delete[] tuple;

    type = reference.type;
    
    if (type == PointerCopy) ptrCopy = reference.ptrCopy;
    else if (type >= PointerReference) ptrRef = reference.ptrRef;
    else {
        auto n = getTupleSize();
        tuple = new Reference[n];
        for (unsigned long i = 0; i < n; i++)
            tuple[i] = reference.tuple[i];
    }
    
    return *this;
}

void Reference::unuse() {
    if (isPointerCopy() && ptrCopy->references == 0)
        delete ptrCopy;
    else if (isTuple()) {
        auto n = getTupleSize();
        for (unsigned long i = 0; i < n; i++)
            tuple[i].unuse();
    }
}