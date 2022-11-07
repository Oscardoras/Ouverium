#ifndef __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__

#include "../Interpreter.hpp"


namespace Array {

    std::shared_ptr<Expression> get_array_size();
    Reference get_array_size(FunctionContext & context);

    std::shared_ptr<Expression> get_array_element();
    Reference get_array_element(FunctionContext & context);

    std::shared_ptr<Expression> get_array_capacity();
    Reference get_array_capacity(FunctionContext & context);

    std::shared_ptr<Expression> set_array_capacity();
    Reference set_array_capacity(FunctionContext & context);

    std::shared_ptr<Expression> add_array_element();
    Reference add_array_element(FunctionContext & context);

    std::shared_ptr<Expression> remove_array_element();
    Reference remove_array_element(FunctionContext & context);

    void init(Context & context);

}


#endif
