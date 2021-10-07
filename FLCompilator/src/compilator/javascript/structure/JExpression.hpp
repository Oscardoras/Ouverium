#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JEXPRESSION_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JEXPRESSION_HPP_

#include <string>
#include <memory>


class JExpression {

public:

    virtual std::string getType() const = 0;

};


#endif