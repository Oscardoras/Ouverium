#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JARRAY_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JARRAY_HPP_

#include <vector>

#include "JExpression.hpp"


class JArray: public JExpression {

public:

    std::vector<std::shared_ptr<JExpression>> objects;

    virtual std::string getType() const {
        return "JArray";
    }

};


#endif