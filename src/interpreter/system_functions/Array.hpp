#ifndef __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__

#include "../Interpreter.hpp"


namespace Interpreter {

    namespace Array {

        Reference get_array_size(FunctionContext & context);

        Reference get_array_element(FunctionContext & context);

        Reference get_array_capacity(FunctionContext & context);

        Reference set_array_capacity(FunctionContext & context);

        Reference add_array_element(FunctionContext & context);

        Reference remove_array_element(FunctionContext & context);

        void init(Context & context);

    }

}


#endif
