#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JVARIABLE_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JVARIABLE_HPP_

#include <vector>

#include "JExpression.hpp"

class JVariable: public JExpression {

public:

    std::string variableName;
    std::shared_ptr<JVariable> next = nullptr;
    std::vector<int> index;

    JVariable getLastVariable() const {
        if (next == nullptr) return *this;
        else return next->getLastVariable();
    }

    virtual std::string getType() const {
        return "Variable";
    }

};


#endif