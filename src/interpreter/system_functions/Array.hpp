#ifndef __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__

#include "../Interpreter.hpp"


namespace Interpreter {

    namespace Array {

        Reference lenght(FunctionContext & context);

        Reference get_capacity(FunctionContext & context);

        Reference set_capacity(FunctionContext & context);

        Reference get(FunctionContext & context);

        Reference add(FunctionContext & context);

        Reference remove(FunctionContext & context);

        void init(Context & context);

    }

}


#endif
