#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include "Context.hpp"
#include "InterpreterError.hpp"

#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"


namespace Interpreter {

    Reference callFunction(Context & context, Function* function, std::shared_ptr<Expression> arguments);

    Reference execute(Context & context, std::shared_ptr<Expression> expression);

    void run(std::shared_ptr<Expression> expression);

}

#endif