#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JFUNCEVAL_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JFUNCEVAL_HPP_

#include <vector>

#include "JExpression.hpp"
#include "JInstruction.hpp"


class JFuncEval: public JExpression {

public:

    std::shared_ptr<JExpression> function;
    std::vector<std::shared_ptr<JExpression>> parameters;

    virtual std::string getType() const {
        return "JFuncEval";
    }

};


#endif