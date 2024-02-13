#ifndef __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_ARRAY_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace Array {

        Reference length(FunctionContext& context);

        Reference get_capacity(FunctionContext& context);

        Reference set_capacity(FunctionContext& context);

        Reference get(FunctionContext& context);

        Reference add(FunctionContext& context);

        Reference remove(FunctionContext& context);

        Reference copy(FunctionContext& context);

        Reference foreach(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
