#include "Function.hpp"


CustomFunction::CustomFunction(std::shared_ptr<FunctionDefinition> pointer) {
    type = Custom;
    this->pointer = pointer;
}

SystemFunction::SystemFunction(std::shared_ptr<Expression> parameters, Reference (*pointer)(FunctionContext&)) {
    type = System;
    this->parameters = parameters;
    this->pointer = pointer;
}