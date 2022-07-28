#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Function.hpp"

#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"



namespace Interpreter {
    
    struct Error {};
    struct FunctionArgumentsError: public Error {};

    Reference call_function(Context & context, std::shared_ptr<Position> position, std::list<Function*> function, Reference reference);

    Reference call_function(Context & context, std::shared_ptr<Position> position, std::list<Function*> function, std::shared_ptr<Expression> arguments);

    Reference execute(Context & context, std::shared_ptr<Expression> expression);

    Reference run(Context & context, std::shared_ptr<Expression> expression);

    bool print(std::ostream & stream, Object* object);

}

#endif
