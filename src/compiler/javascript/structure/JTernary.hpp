#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JTERNARY_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JTERNARY_HPP_

#include <vector>

#include "JExpression.hpp"


class JTernary: public JExpression {

public:

    std::shared_ptr<JExpression> condition;
    std::shared_ptr<JExpression> expression;
    std::shared_ptr<JExpression> alternative;

    virtual std::string getType() const {
        return "JTernary";
    }

};


#endif