#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JEXPRESSION_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JEXPRESSION_HPP_

#include "JInstruction.hpp"


class JExpression: public JInstruction {

public:

    virtual std::string getType() const = 0;

};


#endif