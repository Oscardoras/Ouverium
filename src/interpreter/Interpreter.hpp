#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Function.hpp"


namespace Interpreter {

    struct Error {};
    struct FunctionArgumentsError: public Error {};

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<std::unique_ptr<Function>> const& functions, Reference const& reference);

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<std::unique_ptr<Function>> const& functions, std::shared_ptr<Expression> arguments);

    Reference execute(Context & context, std::shared_ptr<Expression> expression);

    Reference run(Context & context, std::shared_ptr<Expression> expression);

    bool print(std::ostream & stream, Object* object);

}

#endif
