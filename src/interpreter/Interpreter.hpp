#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Function.hpp"


namespace Interpreter {

    class Error: public std::exception {};
    class FunctionArgumentsError: public Error {};

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, Reference const& arguments);

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, std::shared_ptr<Expression> arguments);

    Reference execute(Context & context, std::shared_ptr<Expression> expression);

    Reference run(Context & context, std::shared_ptr<Expression> expression);

    bool print(std::ostream & stream, Data data);

}

#endif
