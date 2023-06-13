#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include <functional>

#include "Object.hpp"


namespace Interpreter {

    class Context;
    class Reference;

    using TupleReference = std::vector<Reference>;
    using SymbolReference = std::reference_wrapper<Data>;
    struct PropertyReference {
        std::reference_wrapper<Object> parent;
        std::reference_wrapper<Data> reference;

        inline operator Data &() const {
            return reference;
        }
    };
    struct ArrayReference {
        std::reference_wrapper<Object> array;
        size_t i;

        inline operator Data &() const {
            return array.get().array[i];
        }
    };

    struct IndirectReference : public std::variant<SymbolReference, PropertyReference, ArrayReference> {

        using std::variant<SymbolReference, PropertyReference, ArrayReference>::variant;

        operator Data &() const;

    };

    struct Reference : public std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference> {

        using std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference>::variant;
        Reference(IndirectReference const& indirect_reference);

        Data to_data(Context & context) const;
        IndirectReference to_indirect_reference(Context & context) const;

    };

}


#endif
