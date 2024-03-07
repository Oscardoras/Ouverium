#ifndef __INTERPRETER_SYSTEMFUNCTIONS_TYPES_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_TYPES_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace Types {

        Reference char_constructor(FunctionContext& context);
        Reference float_constructor(FunctionContext& context);
        Reference int_constructor(FunctionContext& context);
        Reference bool_constructor(FunctionContext& context);
        Reference array_constructor(FunctionContext& context);
        Reference tuple_constructor(FunctionContext& context);
        Reference function_constructor(FunctionContext& context);


        bool check_type(Context& context, Object* object, Object* type);

        Reference is_type(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
