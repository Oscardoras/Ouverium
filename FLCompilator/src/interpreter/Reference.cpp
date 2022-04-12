#include "Context.hpp"
#include "Reference.hpp"


Reference::Reference() {
    type = Pointer;
    pointer = nullptr;
}

Reference::Reference(Reference const& reference) {
    type = reference.type;
    
    if (type == SymbolReference) symbolReference = reference.symbolReference;
    else if (type == PropertyReference) propertyRefrence = reference.propertyRefrence;
    else if (type == ArrayReference) arrayReference = reference.arrayReference;
    else if (type == Pointer) pointer = reference.pointer;
    else {
        tuple = new Reference[type];
        for (long i = 0; i < type; i++)
            tuple[i] = reference.tuple[i];
    }
}

Reference::Reference(Object** const& reference) {
    type = SymbolReference;
    symbolReference = reference;
}

Reference::Reference(Object* const& parent, Object** const& reference) {
    type = PropertyReference;
    propertyRefrence.parent = parent;
    propertyRefrence.reference = reference;
}

Reference::Reference(Object* const& array, unsigned long const& i) {
    type = ArrayReference;
    arrayReference.array = array;
    arrayReference.i = i;
}

Reference::Reference(Object* const& pointer) {
    type = Pointer;
    this->pointer = pointer;
}

Reference::Reference(size_t const& tuple_size) {
    type = tuple_size;
    tuple = new Reference[tuple_size];
}

Reference::~Reference() {
    if (type > 0)
        delete[] tuple;
}

bool Reference::isReference() const {
    return type < 0;
}

Object* & Reference::getReference() const {
    if (type == PropertyReference) return *propertyRefrence.reference;
    else if (type == ArrayReference) return arrayReference.array->data.a[arrayReference.i].o;
    else return *symbolReference;
}

Reference Reference::toSymbolReference(Context & context) const {
    if (type > 0) {
        auto reference = Reference(context.addObject(new Object((size_t) type)));
        for (long i = 0; i < type; i++)
            reference.pointer->data.a[i+1].o = tuple[i].toObject(context);
        return reference;
    } else return *this;
}

Object* Reference::toObject(Context & context) const {
    if (type == Pointer) return pointer;
    else if (isReference()) return getReference();
    else {
        auto object = context.addObject(new Object((size_t) type));
        for (long i = 0; i < type; i++)
            object->data.a[i+1].o = tuple[i].toObject(context);
        return object;
    }
}

Reference& Reference::operator=(Reference const& reference) {
    if (type > 0)
        delete[] tuple;

    type = reference.type;
    
    if (type == SymbolReference) symbolReference = reference.symbolReference;
    else if (type == PropertyReference) propertyRefrence = reference.propertyRefrence;
    else if (type == ArrayReference) arrayReference = reference.arrayReference;
    else if (type == Pointer) pointer = reference.pointer;
    else {
        tuple = new Reference[type];
        for (long i = 0; i < type; i++)
            tuple[i] = reference.tuple[i];
    }
    
    return *this;
}