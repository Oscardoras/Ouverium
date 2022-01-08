#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JVARIABLE_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JVARIABLE_HPP_

#include <vector>

#include "JExpression.hpp"

class JVariable: public JExpression {

public:

    std::string name;

    virtual std::string getType() const {
        return "JVariable";
    }

};


#endif