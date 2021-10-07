#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JASSIGNMENT_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JASSIGNMENT_HPP_

#include "JInstruction.hpp"
#include "JVariable.hpp"


class JAssignment: public JInstruction {

public:

    bool initialization;
    std::shared_ptr<JVariable> variable;
    std::shared_ptr<JExpression> value;

    virtual std::string getType() const {
        return "Assignment";
    }

};


#endif