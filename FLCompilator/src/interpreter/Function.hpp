#ifndef INTERPRETER_FUNCTION_HPP_
#define INTERPRETER_FUNCTION_HPP_

#include <map>
#include <memory>
#include <string>

#include "../parser/expression/FunctionDefinition.hpp"

class Object;
class Reference;
class FunctionContext;


struct Function {
    enum FunctionType {
        Custom,
        System
    } type;

    virtual ~Function() = default;
};

struct CustomFunction: public Function {
    std::shared_ptr<FunctionDefinition> pointer;
    std::map<std::string, Object*> objects;

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