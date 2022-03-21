#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include "../parser/expression/Expression.hpp"


namespace Interpreter {

    Reference callFunction(Context & context, Function* function, std::shared_ptr<Expression> arguments);

    Reference execute(Context & context, std::shared_ptr<Expression> expression);

    void run(std::shared_ptr<Expression> expression);

}

#endif