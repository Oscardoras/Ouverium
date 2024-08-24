#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Context.hpp"
#include "GC.hpp"


namespace Interpreter {

    namespace SystemFunctions {

        void init(GlobalContext& context);

    }

    class Exception {

    public:

        Reference reference;
        std::vector<Parser::Position> positions;

        Exception(Context& context, std::shared_ptr<Parser::Expression> thrower, Reference const& reference);
        Exception(Context& context, std::shared_ptr<Parser::Expression> thrower, std::string const& message);

        void print_stack_trace(Context& context) const;

    };
    class FunctionArgumentsError {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    std::variant<Reference, Exception> try_call_function(Context& context, std::shared_ptr<Parser::Expression> caller, Reference const& func, Arguments const& arguments);
    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> caller, Reference const& func, Arguments const& arguments);

    Reference execute(Context& context, std::shared_ptr<Parser::Expression> expression);

    Reference set(Context& context, Reference const& var, Reference const& data);
    std::string string_from(Context& context, Reference const& data);

}

#endif
