#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include "Object.hpp"


namespace Interpreter {

    class Context;
    class Reference;

    using SymbolReference = Data*;
    struct PropertyReference {
        Object* parent;
        Data* reference;
    };
    struct ArrayReference {
        Object* array;
        size_t i;
    };
    using TupleReference = std::vector<Reference>;

    struct Reference : public std::variant<Data, SymbolReference, PropertyReference, ArrayReference, TupleReference> {

        using std::variant<Data, SymbolReference, PropertyReference, ArrayReference, TupleReference>::variant;

        Data to_data(Context & context) const;
        SymbolReference to_symbol_reference(Context & context);

    };

}


#endif
