#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Function.hpp"


namespace Interpreter {

    class Error: public std::exception {};
    class FunctionArgumentsError: public Error {};

    Reference call_function(Context & context, std::shared_ptr<Parser::Expression> expression, std::list<Function> const& functions, std::shared_ptr<Parser::Expression> arguments);
    Reference call_function(Context & context, std::shared_ptr<Parser::Expression> expression, std::list<Function> const& functions, Reference const& arguments);

    Reference execute(Context & context, std::shared_ptr<Parser::Expression> expression);

    Reference run(Context & context, std::shared_ptr<Parser::Expression> expression);

    bool print(Context & context, std::ostream & stream, Data data);

}

#endif
