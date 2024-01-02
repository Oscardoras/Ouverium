#ifndef __INTERPRETER_SYSTEMFUNCTIONS_DLL_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_DLL_HPP__

#include <filesystem>

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace Dll {

        std::filesystem::path get_canonical_path(FunctionContext& context);

        Reference import(FunctionContext & context);

        Reference import_dll(FunctionContext& context);

        Reference dlopen(FunctionContext& context);

        Reference extern_statement(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
