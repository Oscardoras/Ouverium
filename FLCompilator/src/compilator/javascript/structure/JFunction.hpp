#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JFUNCTION_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JFUNCTION_HPP_

#include <vector>

#include "JExpression.hpp"
#include "JInstruction.hpp"
#include "JVariable.hpp"


class JFunction: public JExpression {

public:

    std::vector<std::shared_ptr<JVariable>> parameters;
    std::vector<std::shared_ptr<JInstruction>> instructions;

    virtual std::string getType() const {
        return "Function";
    }

};


#endif