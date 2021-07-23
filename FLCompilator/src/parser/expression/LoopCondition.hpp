#ifndef PARSER_EXPRESSION_LOOPCONDITION_HPP_
#define PARSER_EXPRESSION_LOOPCONDITION_HPP_

#include "Condition.hpp"


class LoopCondition: public Condition {

public:

    virtual std::string getType() const {
        return "LoopCondition";
    }

};


#endif