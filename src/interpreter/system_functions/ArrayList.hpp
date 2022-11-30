#ifndef __INTERPRETER_SYSTEMFUNCTIONS_ARRAYLIST_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_ARRAYLIST_HPP__

#include "Array.hpp"


namespace Interpreter {

    namespace ArrayList {

        Reference array_list(FunctionContext & context);

        Reference get_array_element(FunctionContext & context);

        Reference foreach(FunctionContext & context);

        Reference lenght(FunctionContext & context);

        Reference is_empty(FunctionContext & context);

        Reference get_capacity(FunctionContext & context);

        Reference set_capacity(FunctionContext & context);

        Reference get_first(FunctionContext & context);

        Reference get_last(FunctionContext & context);

        Reference add_first(FunctionContext & context);

        Reference add_last(FunctionContext & context);

        Reference remove_first(FunctionContext & context);

        Reference remove_last(FunctionContext & context);

        Reference insert(FunctionContext & context);

        Reference remove(FunctionContext & context);

    /*
        Reference iterator_first(FunctionContext & context);

        Reference iterator_last(FunctionContext & context);

        Reference iterator(FunctionContext & context);

        Reference iterator_index(FunctionContext & context);
    */

        void init(Context & context);

    }

}


#endif
