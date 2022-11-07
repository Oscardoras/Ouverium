#ifndef __INTERPRETER_SYSTEMFUNCTIONS_STRING_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_STRING_HPP__

#include "ArrayList.hpp"


namespace String {

    std::shared_ptr<Expression> string();
    Reference char_constructor(FunctionContext & context);

    std::shared_ptr<Expression> char_is();

    Reference char_is_digit(FunctionContext & context);
    Reference char_is_alpha(FunctionContext & context);
    Reference char_is_alphanum(FunctionContext & context);

    Reference string(FunctionContext & context);

    std::shared_ptr<Expression> index_of();
    Reference index_of(FunctionContext & context);

    std::shared_ptr<Expression> substring();
    Reference substring(FunctionContext & context);

    std::shared_ptr<Expression> includes();
    Reference includes(FunctionContext & context);

    std::shared_ptr<Expression> concat();
    Reference concat(FunctionContext & context);

    std::shared_ptr<Expression> assign_concat();
    Reference assign_concat(FunctionContext & context);

    void init(Context & context);

}


#endif
