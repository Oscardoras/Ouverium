#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JWHILE_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JWHILE_HPP_

#include <vector>

#include "JExpression.hpp"
#include "JInstruction.hpp"


class JWhile: public JInstruction {

public:

    std::shared_ptr<JExpression> condition;
    std::vector<std::shared_ptr<JInstruction>> instructions;

    virtual std::string getType() const {
        return "JWhile";
    }

};


#endif