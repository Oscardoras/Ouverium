#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Function.hpp"

#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"


struct InterpreterError {};
struct FunctionArgumentsError {};

namespace Interpreter {

    Reference call_function(Context & context, std::list<Function*> function, std::shared_ptr<Expression> arguments, std::shared_ptr<Position> position);

    Reference execute(Context & context, std::shared_ptr<Expression> expression);

    void set_standard_context(Context & context);

    Reference run(Context & context, std::string const& path, std::string const& code);

    bool print(std::ostream & stream, Object* object);

}

#endif
