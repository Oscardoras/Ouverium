#ifndef __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        Reference read(FunctionContext& context);
        Reference has(FunctionContext& context);
        Reference write(FunctionContext& context);
        Reference flush(FunctionContext& context);
        Reference open(FunctionContext& context);
        Reference working_directory(FunctionContext& context);

        Reference time(FunctionContext& context);

        Reference weak_reference(FunctionContext& context);
        Reference GC_collect(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
