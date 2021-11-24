#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JPROPERTYCALL_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JPROPERTYCALL_HPP_

#include <vector>

#include "JExpression.hpp"

class JPropertyCall: public JExpression {

public:

    std::shared_ptr<JExpression> object;
    std::shared_ptr<JExpression> property;

    virtual std::string getType() const {
        return "JPropertyCall";
    }

};


#endif