#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include <functional>
#include <vector>

#include "Data.hpp"

#include "../parser/Expressions.hpp"


namespace Interpreter {

    class Context;
    struct Object;
    class Reference;

    using TupleReference = std::vector<Reference>;
    using SymbolReference = std::reference_wrapper<Data>;
    struct PropertyReference {
        Data parent;
        std::string name;
    };
    struct ArrayReference {
        Data array;
        size_t i;
    };

    class IndirectReference : public std::variant<SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<SymbolReference, PropertyReference, ArrayReference>::variant;

        Data to_data(Context& context, std::shared_ptr<Parser::Expression> caller = nullptr) const;

    };

    class Reference : public std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<Data, TupleReference, SymbolReference, PropertyReference, ArrayReference>::variant;
        Reference(IndirectReference const& indirect_reference);

        Data to_data(Context& context, std::shared_ptr<Parser::Expression> caller = nullptr) const;
        IndirectReference to_indirect_reference(Context& context, std::shared_ptr<Parser::Expression> caller = nullptr) const;

    };

    inline auto operator==(SymbolReference const& a, SymbolReference const& b) {
        return &a.get() == &b.get();
    }

    inline auto operator==(PropertyReference const& a, PropertyReference const& b) {
        return a.parent == b.parent && a.name == b.name;
    }

    inline auto operator==(ArrayReference const& a, ArrayReference const& b) {
        return a.array == b.array && a.i == b.i;
    }

}


#endif
