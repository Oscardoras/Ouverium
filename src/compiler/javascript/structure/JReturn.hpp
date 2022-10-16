#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JRETURN_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JRETURN_HPP_

#include "JExpression.hpp"
#include "JInstruction.hpp"


class JReturn: public JInstruction {

public:

    std::shared_ptr<JExpression> value;

    virtual std::string getType() const {
        return "JReturn";
    }

};


#endif