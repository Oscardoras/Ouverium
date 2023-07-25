#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include <functional>

#include "Data.hpp"


namespace Interpreter {

    class Context;
    class Object;
    class Reference;

    using TupleReference = std::vector<Reference>;
    using SymbolReference = std::reference_wrapper<Data>;
    struct PropertyReference {
        std::reference_wrapper<Object> parent;
        std::string name;

        operator Data &() const;
    };
    struct ArrayReference {
        std::reference_wrapper<Object> array;
        size_t i;

        operator Data &() const;
    };

    class IndirectReference : public std::variant<SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<SymbolReference, PropertyReference, ArrayReference>::variant;

        Data to_data(Context & context) const;

    };

    class Reference : public std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference>::variant;
        Reference(IndirectReference const& indirect_reference);

        Data to_data(Context & context) const;
        IndirectReference to_indirect_reference(Context & context) const;

    };
    bool operator<(SymbolReference const& a, SymbolReference const& b);
    bool operator<(PropertyReference const& a, PropertyReference const& b);
    bool operator<(ArrayReference const& a, ArrayReference const& b);

    struct Comparator {
        bool operator()(Reference const& a, Reference const& b) const {
            return a < b;
        }
    };

}


#endif
