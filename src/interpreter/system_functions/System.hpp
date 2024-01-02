#ifndef __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        Reference time(FunctionContext& context);

        Reference random(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
