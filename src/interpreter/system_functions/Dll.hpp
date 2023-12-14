#ifndef __INTERPRETER_SYSTEMFUNCTIONS_DLL_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_DLL_HPP__

#include "../Interpreter.hpp"


namespace Interpreter {

    namespace Dll {

        Reference include(FunctionContext & context);

        Reference extern_function(FunctionContext & context);

        Reference open(FunctionContext & context);

        void init(GlobalContext & context);

    }

}


#endif
