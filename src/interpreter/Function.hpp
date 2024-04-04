#ifndef __INTERPRETER_FUNCTION_HPP__
#define __INTERPRETER_FUNCTION_HPP__

#include <functional>
#include <map>

#include "Reference.hpp"


namespace Interpreter {

    class FunctionContext;

    using CustomFunction = std::shared_ptr<Parser::FunctionDefinition>;

    struct SystemFunction {
        std::shared_ptr<Parser::Expression> parameters;
        std::function<Reference(FunctionContext&)> pointer;
    };

    struct Function : public std::variant<CustomFunction, SystemFunction> {
        std::map<std::string, IndirectReference> extern_symbols;

        using std::variant<CustomFunction, SystemFunction>::variant;
    };

}


#endif
