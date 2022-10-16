#ifndef __COMPILER_C_TRANSLATOR_HPP_
#define __COMPILER_C_TRANSLATOR_HPP_

#include "../../parser/expression/Expression.hpp"


namespace CTranslator {

    struct Symbol {

    };

    struct Object {
        std::shared_ptr<Expression> creation;
        bool is_is_stack;
        std::vector<std::string> properties;
    };

}


#endif
