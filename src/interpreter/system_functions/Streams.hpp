#ifndef __INTERPRETER_SYSTEMFUNCTIONS_STREAMS_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_STREAMS_HPP__

#include "../Interpreter.hpp"


namespace Interpreter {

    namespace Streams {

        Reference print(FunctionContext& context);

        Reference scan(FunctionContext& context);

        Reference read(FunctionContext& context);

        Reference has(FunctionContext& context);

        void setInputStream(Context& context, Object& object);

        Reference write(FunctionContext& context);

        Reference flush(FunctionContext& context);

        void setOutputStream(Context& context, Object& object);

        Reference input_file(FunctionContext& context);
        Reference output_file(FunctionContext& context);

        Reference wd(FunctionContext& context);

        void init(GlobalContext& context);

    }

}


#endif
