#ifndef PARSER_EXPRESSION_CONDITIONREPEAT_HPP_
#define PARSER_EXPRESSION_CONDITIONREPEAT_HPP_

#include "Condition.hpp"


class ConditionRepeat: public Condition {

public:

    virtual std::string getType() const {
        return "ConditionRepeat";
    }

};


#endif