#ifndef __INTERPRETER_FUNCTION_HPP__
#define __INTERPRETER_FUNCTION_HPP__

#include <map>
#include <memory>

#include "Reference.hpp"

#include "../parser/Expressions.hpp"


namespace Interpreter {

    class FunctionContext;

    using CustomFunction = std::shared_ptr<Parser::FunctionDefinition>;

    struct SystemFunction {
        std::shared_ptr<Parser::Expression> parameters;
        Reference(*pointer)(FunctionContext&);
    };

    struct Function : public std::variant<CustomFunction, SystemFunction> {
        std::map<std::string, IndirectReference> extern_symbols;

        using std::variant<CustomFunction, SystemFunction>::variant;
    };

}


#endif
