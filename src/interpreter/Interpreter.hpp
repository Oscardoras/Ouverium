#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include <iostream>

#include "Function.hpp"


namespace Interpreter {

    class Exception {

    public:

        Reference reference;
        std::vector<std::shared_ptr<Parser::Position>> positions;

        Exception(Context & context, Reference const& reference, std::shared_ptr<Parser::Expression> expression);
        Exception(Context & context, std::string const& message, Data const& type, std::shared_ptr<Parser::Expression> expression);

    };
    class FunctionArgumentsError {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    Reference call_function(Context & context, std::shared_ptr<Parser::Expression> expression, std::list<Function> const& functions, Arguments const& arguments);
    Reference execute(Context & context, std::shared_ptr<Parser::Expression> expression);

    Reference set(Context & context, Reference const& var, Reference const& data);
    std::string string_from(Context & context, Reference const& data);

}

#endif
