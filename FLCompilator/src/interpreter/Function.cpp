#include "Function.hpp"


CustomFunction::CustomFunction(std::shared_ptr<FunctionDefinition> pointer) {
    type = Custom;
    this->pointer = pointer;
}

SystemFunction::SystemFunction(Reference (*pointer)(Reference, FunctionContext&)) {
    type = System;
    this->pointer = pointer;
}