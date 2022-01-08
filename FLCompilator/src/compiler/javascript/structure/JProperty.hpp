#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JPROPERTY_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JPROPERTY_HPP_

#include <vector>

#include "JExpression.hpp"

class JProperty: public JExpression {

public:

    std::shared_ptr<JExpression> object;
    std::string name;

    virtual std::string getType() const {
        return "JProperty";
    }

};


#endif