#ifndef __INTERPRETER_SYSTEMFUNCTIONS_MATH_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_MATH_HPP__

#include "../Interpreter.hpp"


namespace Math {

    std::shared_ptr<Expression> a();
    std::shared_ptr<Expression> ab();
    std::shared_ptr<Expression> a_b();

    Reference logical_not(FunctionContext & context);

    Reference logical_and(FunctionContext & context);

    Reference logical_or(FunctionContext & context);

    Reference addition(FunctionContext & context);

    Reference opposite(FunctionContext & context);

    Reference substraction(FunctionContext & context);

    Reference multiplication(FunctionContext & context);

    Reference division(FunctionContext & context);

    Reference modulo(FunctionContext & context);

    Reference strictly_inf(FunctionContext & context);

    Reference strictly_sup(FunctionContext & context);
    
    Reference inf_equals(FunctionContext & context);

    Reference sup_equals(FunctionContext & context);

    Reference increment(FunctionContext & context);

    Reference decrement(FunctionContext & context);

    Reference add(FunctionContext & context);

    Reference remove(FunctionContext & context);

    Reference mutiply(FunctionContext & context);

    Reference divide(FunctionContext & context);

    void init(Context & context);

}


#endif
