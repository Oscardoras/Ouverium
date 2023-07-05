#ifndef __INTERPRETER_SYSTEMFUNCTIONS_TYPES_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_TYPES_HPP__

#include "../Interpreter.hpp"


namespace Interpreter {

    namespace Types {

        bool check_type(Context & context, Object* object, Object* type);

        Reference is_type(FunctionContext & context);

        Reference set_type(FunctionContext & context);

        void init(GlobalContext & context);

    }

}


#endif
