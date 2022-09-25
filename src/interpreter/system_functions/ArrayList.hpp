#ifndef __INTERPRETER_SYSTEMFUNCTIONS_ARRAYLIST_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_ARRAYLIST_HPP__

#include "Array.hpp"


namespace ArrayList {

    std::shared_ptr<Expression> array_list();
    Reference array_list(FunctionContext & context);

    Reference get_array_element(FunctionContext & context);

    std::shared_ptr<Expression> foreach();
    Reference foreach(FunctionContext & context);

    Reference lenght(FunctionContext & context);

    Reference is_empty(FunctionContext & context);

    Reference get_capacity(FunctionContext & context);

    std::shared_ptr<Expression> set_capacity();
    Reference set_capacity(FunctionContext & context);

    Reference get_first(FunctionContext & context);

    Reference get_last(FunctionContext & context);

    std::shared_ptr<Expression> add_first();
    Reference add_first(FunctionContext & context);

    std::shared_ptr<Expression> add_last();
    Reference add_last(FunctionContext & context);

    Reference remove_first(FunctionContext & context);

    Reference remove_last(FunctionContext & context);

    std::shared_ptr<Expression> insert();
    Reference insert(FunctionContext & context);

    std::shared_ptr<Expression> remove();
    Reference remove(FunctionContext & context);

    Reference iterator_first(FunctionContext & context);

    Reference iterator_last(FunctionContext & context);

    Reference iterator(FunctionContext & context);

    std::shared_ptr<Expression> iterator_index();
    Reference iterator_index(FunctionContext & context);

    void init(Context & context);

}


#endif
