#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include <functional>
#include <vector>

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

        operator Data& () const;
    };
    struct ArrayReference {
        std::reference_wrapper<Object> array;
        size_t i;

        operator Data& () const;
    };

    class IndirectReference : public std::variant<SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<SymbolReference, PropertyReference, ArrayReference>::variant;

        Data to_data(Context& context) const;

    };

    class Reference : public std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference> {

    protected:

        Data get_data(Context& context) const;
        friend Data compute(Context&, Reference const&, Data const&);

    public:

        using std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference>::variant;
        Reference(IndirectReference const& indirect_reference);

        Data to_data(Context& context) const;
        IndirectReference to_indirect_reference(Context& context) const;

    };

    inline auto operator==(SymbolReference const& a, SymbolReference const& b) {
        return &a.get() == &b.get();
    }

    inline auto operator==(PropertyReference const& a, PropertyReference const& b) {
        return &a.parent.get() == &b.parent.get() && a.name == b.name;
    }

    inline auto operator==(ArrayReference const& a, ArrayReference const& b) {
        return &a.array.get() == &b.array.get() && a.i == b.i;
    }

}


#endif
