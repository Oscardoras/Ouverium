#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include <iostream>

#include "Function.hpp"


namespace Interpreter {

    class Exception {

    public:

        Reference reference;
        std::shared_ptr<Parser::Position> position;

    };
    class FunctionArgumentsError {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    Reference call_function(Context & context, std::shared_ptr<Parser::Expression> expression, std::list<Function> const& functions, Arguments const& arguments);
    Reference execute(Context & context, std::shared_ptr<Parser::Expression> expression);

    Reference run(Context & context, std::shared_ptr<Parser::Expression> expression);
    Reference set(Context & context, Reference const& var, Reference const& data);
    std::string to_string(Context & context, Reference const& data);
    Exception get_exception(Context & context, std::string const& message, Data const& type, std::shared_ptr<Parser::Expression> expression);

}

#endif
