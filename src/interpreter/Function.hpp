#ifndef __INTERPRETER_FUNCTION_HPP__
#define __INTERPRETER_FUNCTION_HPP__

#include <map>
#include <memory>
#include <string>

#include "Context.hpp"


namespace Interpreter {

    struct Function {

        enum FunctionType {
            Custom,
            System
        } type;

        std::map<std::string, Reference> extern_symbols;

        inline Function() = default;

        inline Function(Function const& function) = default;

        virtual ~Function() = default;

    };

    struct CustomFunction: public Function {

        std::shared_ptr<FunctionDefinition> pointer;

        inline CustomFunction(CustomFunction const& function) = default;

        inline CustomFunction(std::shared_ptr<FunctionDefinition> pointer) {
            type = Custom;
            this->pointer = pointer;
        }

    };

    struct SystemFunction: public Function {

        std::shared_ptr<Expression> parameters;
        Reference (*pointer)(FunctionContext&);

        inline SystemFunction(SystemFunction const& function) = default;

        inline SystemFunction(std::shared_ptr<Expression> parameters, Reference (*pointer)(FunctionContext&)) {
            type = System;
            this->parameters = parameters;
            this->pointer = pointer;
        }

    };

}


#endif
