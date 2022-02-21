#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include "../parser/expression/Expression.hpp"


namespace Interpreter {

    void run(std::shared_ptr<Expression> expression);

}

#endif