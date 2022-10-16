#ifndef __COMPILER_JAVASCRIPT_STRUCTURE_JARRAY_HPP__
#define __COMPILER_JAVASCRIPT_STRUCTURE_JARRAY_HPP__

#include <vector>

#include "JExpression.hpp"


struct JArray: public JExpression {

    JArray() {
        type = Type::Array;
    }

    std::vector<std::shared_ptr<JExpression>> objects;

};


#endif
