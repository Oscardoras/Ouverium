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

    FunctionDefinition* pointer;

    inline CustomFunction(FunctionDefinition* pointer) {
        type = Custom;
        this->pointer = pointer;
    }

};

struct SystemFunction: public Function {

    FunctionDefinition* parameters;
    Reference (*pointer)(FunctionContext&);

    inline SystemFunction(FunctionDefinition* parameters, Reference (*pointer)(FunctionContext&)) {
        type = System;
        this->parameters = parameters;
        this->pointer = pointer;
    }

};


#endif
