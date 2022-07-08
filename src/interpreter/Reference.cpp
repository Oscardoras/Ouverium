#include "Context.hpp"
#include "Reference.hpp"


Reference::Reference() {
    type = Pointer;
    pointer = nullptr;
}

Reference::Reference(Reference const& reference) {
    type = reference.type;
    
    if (type == SymbolReference) symbol_reference = reference.symbol_reference;
    else if (type == PropertyReference) property_reference = reference.property_reference;
    else if (type == ArrayReference) array_reference = reference.array_reference;
    else if (type == Pointer) pointer = reference.pointer;
    else {
        tuple = new Reference[type];
        for (long i = 0; i < type; i++)
            tuple[i] = reference.tuple[i];
    }
}

Reference::Reference(Object** const& symbol_reference) {
    type = SymbolReference;
    this->symbol_reference = symbol_reference;
}

Reference::Reference(Object* const& parent, Object** const& reference) {
    type = PropertyReference;
    property_reference.parent = parent;
    property_reference.reference = reference;
}

Reference::Reference(Object* const& array, unsigned long const& i) {
    type = ArrayReference;
    array_reference.array = array;
    array_reference.i = i;
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

bool Reference::is_reference() const {
    return type < 0;
}

Object* & Reference::get_reference() const {
    if (type == PropertyReference) return *property_reference.reference;
    else if (type == ArrayReference) return array_reference.array->data.a[array_reference.i+1].o;
    else return *symbol_reference;
}

Object* Reference::to_object(Context & context) const {
    if (type == Pointer) return pointer;
    else if (is_reference()) return get_reference();
    else {
        auto object = context.new_object((size_t) type);
        for (long i = 0; i < type; i++)
            object->data.a[i+1].o = tuple[i].to_object(context);
        return object;
    }
}

Reference& Reference::operator=(Reference const& reference) {
    if (type > 0)
        delete[] tuple;

    type = reference.type;
    
    if (type == SymbolReference) symbol_reference = reference.symbol_reference;
    else if (type == PropertyReference) property_reference = reference.property_reference;
    else if (type == ArrayReference) array_reference = reference.array_reference;
    else if (type == Pointer) pointer = reference.pointer;
    else {
        tuple = new Reference[type];
        for (long i = 0; i < type; i++)
            tuple[i] = reference.tuple[i];
    }
    
    return *this;
}
