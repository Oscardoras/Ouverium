#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JIF_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JIF_HPP_

#include <vector>

#include "JExpression.hpp"
#include "JInstruction.hpp"


class JIf: public JInstruction {

public:

    std::shared_ptr<JExpression> condition;
    std::vector<std::shared_ptr<JInstruction>> instructions;

    virtual std::string getType() const {
        return "JIf";
    }

};


#endif