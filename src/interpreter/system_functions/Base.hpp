#ifndef __INTERPRETER_SYSTEMFUNCTIONS_BASE_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_BASE_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace Base {

        Reference getter(FunctionContext& context);
        Reference function_getter(FunctionContext& context);
        Reference setter(FunctionContext& context);


        Reference separator(FunctionContext& context);

        Reference if_statement(FunctionContext& context);

        Reference while_statement(FunctionContext& context);

        Reference for_statement(FunctionContext& context);

        Reference for_step_statement(FunctionContext& context);

        Reference try_statement(FunctionContext& context);

        Reference throw_statement(FunctionContext& context);

        Reference copy(FunctionContext& context);
        Reference copy_pointer(FunctionContext& context);

        Reference define(FunctionContext& context);

        Reference function_definition(FunctionContext& context);
        Reference function_add(FunctionContext& context);

        bool eq(Data a, Data b);

        Reference equals(FunctionContext& context);

        Reference not_equals(FunctionContext& context);

        Reference check_pointers(FunctionContext& context);

        Reference not_check_pointers(FunctionContext& context);

        Reference string_from(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
