#include "Function.hpp"


CustomFunction::CustomFunction(std::shared_ptr<FunctionDefinition> pointer) {
    type = Custom;
    this->pointer = pointer;
}

SystemFunction::SystemFunction(Reference (*pointer)(Reference)) {
    type = System;
    this->pointer = pointer;
}