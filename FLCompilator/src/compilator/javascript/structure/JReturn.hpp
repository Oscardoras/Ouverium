#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JRETURN_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JRETURN_HPP_

#include "JExpression.hpp"
#include "JInstruction.hpp"


class JReturn: public JInstruction {

public:

    std::shared_ptr<JExpression> value;

    virtual std::string getType() const {
        return "Return";
    }

};


#endif