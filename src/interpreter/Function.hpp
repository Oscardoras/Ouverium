#ifndef __INTERPRETER_FUNCTION_HPP__
#define __INTERPRETER_FUNCTION_HPP__

#include <map>
#include <memory>
#include <string>

#include "Context.hpp"

#include "../parser/expression/FunctionDefinition.hpp"


struct Function {

    enum FunctionType {
        Custom,
        System
    } type;

    std::map<std::string, Reference> extern_symbols;

};

struct CustomFunction: public Function {

    std::shared_ptr<FunctionDefinition> pointer;

    CustomFunction(std::shared_ptr<FunctionDefinition> pointer) {
        type = Custom;
        this->pointer = pointer;
    }

};

struct SystemFunction: public Function {

    std::shared_ptr<Expression> parameters;
    Reference (*pointer)(FunctionContext&);

    SystemFunction(std::shared_ptr<Expression> parameters, Reference (*pointer)(FunctionContext&)) {
        type = System;
        this->parameters = parameters;
        this->pointer = pointer;
    }

};


#endif
