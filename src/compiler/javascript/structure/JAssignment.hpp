#ifndef __COMPILER_JAVASCRIPT_STRUCTURE_JASSIGNMENT_HPP__
#define __COMPILER_JAVASCRIPT_STRUCTURE_JASSIGNMENT_HPP__

#include "JInstruction.hpp"
#include "JVariable.hpp"


struct JAssignment: public JInstruction {

    JAssignment() {
        type = Type::Assignment;
    }

    bool initialization;
    std::shared_ptr<JVariable> variable;
    std::shared_ptr<JExpression> value;

};


#endif