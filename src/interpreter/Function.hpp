#ifndef __INTERPRETER_FUNCTION_HPP__
#define __INTERPRETER_FUNCTION_HPP__

#include <map>
#include <memory>
#include <string>

#include "Context.hpp"


namespace Interpreter {

    struct CustomFunction {
        std::shared_ptr<Parser::FunctionDefinition> pointer;
    };

    struct SystemFunction {
        std::shared_ptr<Parser::Expression> parameters;
        Reference (*pointer)(FunctionContext&);
    };

    struct Function: public std::variant<CustomFunction, SystemFunction> {
        std::map<std::string, IndirectReference> extern_symbols;

        using std::variant<CustomFunction, SystemFunction>::variant;
    };

}


#endif
