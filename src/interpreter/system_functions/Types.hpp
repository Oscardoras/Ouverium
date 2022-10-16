#ifndef __INTERPRETER_SYSTEMFUNCTIONS_TYPES_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_TYPES_HPP__

#include "../Interpreter.hpp"


namespace Types {

    bool check_type(Context & context, Object* object, Object* type);

    std::shared_ptr<Expression> getset_type();

    Reference is_type(FunctionContext & context);

    Reference set_type(FunctionContext & context);

    void init(Context & context);

}


#endif
