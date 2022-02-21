#ifndef INTERPRETER_FUNCTION_HPP_
#define INTERPRETER_FUNCTION_HPP_

#include <map>
#include <memory>
#include <string>

#include "../parser/expression/FunctionDefinition.hpp"

class Object;
class Reference;


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
    Reference (*pointer)(Reference);

    SystemFunction(Reference (*pointer)(Reference));
    virtual ~SystemFunction() = default;
};


#endif