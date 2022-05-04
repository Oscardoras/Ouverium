#ifndef INTERPRETER_FUNCTION_HPP_
#define INTERPRETER_FUNCTION_HPP_

#include <map>
#include <memory>
#include <string>

#include "../parser/expression/FunctionDefinition.hpp"

#include "Context.hpp"


struct Function {
    enum FunctionType {
        Custom,
        System
    } type;
    std::map<std::string, Reference> externSymbols;

    virtual ~Function() = default;
};

struct CustomFunction: public Function {
    std::shared_ptr<FunctionDefinition> pointer;

    CustomFunction(std::shared_ptr<FunctionDefinition> pointer);
    virtual ~CustomFunction() = default;
};

struct SystemFunction: public Function {
    std::shared_ptr<Expression> parameters;
    Reference (*pointer)(FunctionContext&);

    SystemFunction(std::shared_ptr<Expression> parameters, Reference (*pointer)(FunctionContext&));
    virtual ~SystemFunction() = default;
};


#endif