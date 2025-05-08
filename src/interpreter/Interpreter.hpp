#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "Context.hpp" // IWYU pragma: export
#include "Data.hpp" // IWYU pragma: export
#include "Function.hpp" // IWYU pragma: export
#include "GC.hpp" // IWYU pragma: export
#include "Object.hpp" // IWYU pragma: export
#include "Reference.hpp" // IWYU pragma: export

#include "../parser/Expressions.hpp"


namespace Interpreter {

    class Exception {

    public:

        Reference reference;
        std::vector<Parser::Position> positions;

        Exception(Context& context, std::shared_ptr<Parser::Expression> const& thrower, Reference reference);
        Exception(Context& context, std::shared_ptr<Parser::Expression> const& thrower, std::string const& message);

        void print_stack_trace(Context& context) const;

    };
    class FunctionArgumentsError {};

    class Arguments : public std::variant<std::shared_ptr<Parser::Expression>, Reference, std::vector<Arguments>> {
        using std::variant<std::shared_ptr<Parser::Expression>, Reference, std::vector<Arguments>>::variant;
    };

    std::variant<Reference, Exception> try_call_function(Context& context, std::shared_ptr<Parser::Expression> const& caller, Reference const& func, Arguments const& arguments);
    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> const& caller, Reference const& func, Arguments const& arguments);

    Reference execute(Context& context, std::shared_ptr<Parser::Expression> const& expression);

    Reference set(Context& context, Reference const& var, Reference const& data);
    [[nodiscard]] std::string string_from(Context& context, Reference const& data);

}

#endif
