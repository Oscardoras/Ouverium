#include "Context.hpp"
#include "Reference.hpp"


namespace Interpreter {

    Reference::Reference():
    type(Type::Data), data(Data(nullptr)) {}

    Reference::Reference(Reference const& reference):
    type(reference.type) {
        if (type == Type::Symbol) symbol_reference = reference.symbol_reference;
        else if (type == Type::Property) property_reference = reference.property_reference;
        else if (type == Type::Array) array_reference = reference.array_reference;
        else if (type == Type::Data) data = reference.data;
        else {
            tuple_reference.size = reference.tuple_reference.size;
            tuple_reference.tuple = new Reference[tuple_reference.size];
            for (size_t i = 0; i < tuple_reference.size; i++)
                tuple_reference.tuple[i] = reference.tuple_reference.tuple[i];
        }
    }

    Reference::Reference(SymbolReference symbol_reference):
    type(Type::Symbol), symbol_reference(symbol_reference) {}

    Reference::Reference(Object* parent, Data* reference) {
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

    Reference::Reference(size_t tuple_size):
    tuple_reference.size(tuple_size) {
        if (tuple_reference > 0)
            tuple_reference.tuple = new Reference[tuple_size];
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

}
