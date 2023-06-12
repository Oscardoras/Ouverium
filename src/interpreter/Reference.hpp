#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include <functional>

#include "Object.hpp"


namespace Interpreter {

    class Context;
    class Reference;

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
    using TupleReference = std::vector<Reference>;

    struct Reference : public std::variant<Data, SymbolReference, PropertyReference, ArrayReference, TupleReference> {

        using std::variant<Data, SymbolReference, PropertyReference, ArrayReference, TupleReference>::variant;

        Data to_data(Context & context) const;
        Data & to_symbol_reference(Context & context) const;

    protected:

        static Data compute_data(Context & context, Data const& data);

    };

}


#endif
