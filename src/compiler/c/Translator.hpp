#ifndef __COMPILER_C_TRANSLATOR_HPP_
#define __COMPILER_C_TRANSLATOR_HPP_

#include "../../parser/expression/Expression.hpp"
#include "../../parser/expression/FunctionCall.hpp"
#include "../../parser/expression/FunctionDefinition.hpp"
#include "../../parser/expression/Property.hpp"
#include "../../parser/expression/Symbol.hpp"
#include "../../parser/expression/Tuple.hpp"


namespace CTranslator {

    struct Symbol {
        std::string name;
    };

    struct Object {
        std::shared_ptr<Expression> creation;
        bool is_is_stack;
        std::vector<std::string> properties;
    };

    struct Function {
        std::shared_ptr<FunctionDefinition> expression;
        std::vector<Symbol> parameters;
        std::vector<Symbol> local_variables;
    };

    struct Environment {
        std::vector<Symbol> global_variables;
        std::vector<Function> functions;
    };

}


#endif
