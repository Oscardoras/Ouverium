#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JUNDEFINED_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JUNDEFINED_HPP_

#include "JExpression.hpp"

class JUndefined: public JExpression {

public:

    virtual std::string getType() const {
        return "JUndefined";
    }

};


#endif