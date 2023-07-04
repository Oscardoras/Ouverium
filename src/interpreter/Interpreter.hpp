#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Function.hpp"


namespace Interpreter {

    class Error: public std::exception {};
    class FunctionArgumentsError: public Error {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    Reference call_function(Context & context, std::shared_ptr<Parser::Expression> expression, std::list<Function> const& functions, Arguments const& arguments);
    Reference execute(Context & context, std::shared_ptr<Parser::Expression> expression);

    Reference run(Context & context, std::shared_ptr<Parser::Expression> expression);
    bool print(Context & context, std::ostream & stream, Data data);

}

#endif
