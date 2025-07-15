#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

// IWYU pragma: private; include "Interpreter.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <variant>

#include "Data.hpp"

#include "../parser/Expressions.hpp"


namespace Interpreter {

    class Context;
    class Reference;

    using SymbolReference = std::shared_ptr<Data>;
    struct PropertyReference {
        Data parent;
        std::string name;

        friend bool operator==(PropertyReference const& a, PropertyReference const& b) {
            return a.parent == b.parent && a.name == b.name;
        }
    };
    struct ArrayReference {
        Data array;
        size_t i;

        friend bool operator==(ArrayReference const& a, ArrayReference const& b) {
            return a.array == b.array && a.i == b.i;
        }
    };

    class IndirectReference : public std::variant<SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<SymbolReference, PropertyReference, ArrayReference>::variant;
        IndirectReference(Reference reference);

        [[nodiscard]] const Data& get_data() const;
        [[nodiscard]] Data& data();
        [[nodiscard]] Data to_data(Context& context, std::shared_ptr<Parser::Expression> const& caller = nullptr) const;

    };

    class Reference : public std::variant<Data, SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<Data, SymbolReference, PropertyReference, ArrayReference>::variant;
        Reference(IndirectReference indirect_reference);

        [[nodiscard]] Data const& get_data() const;
        [[nodiscard]] Data to_data(Context& context, std::shared_ptr<Parser::Expression> const& caller = nullptr) const;
    };

}


#endif
