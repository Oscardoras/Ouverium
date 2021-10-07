#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JUNDEFINED_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JUNDEFINED_HPP_

#include "JExpression.hpp"

class JUndefined: public JExpression {

public:

    virtual std::string getType() const {
        return "Undefined";
    }

};


#endif