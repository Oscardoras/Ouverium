#ifndef __COMPILER_C_TRANSLATOR_HPP_
#define __COMPILER_C_TRANSLATOR_HPP_

#include <list>

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
        bool is_stack;

        std::map<std::string, std::reference_wrapper<Object>> properties;

        std::list<std::reference_wrapper<Function>> functions;

        enum ObjectType {
            CPointer = -5,
            Char,
            Float,
            Int,
            Bool,
            None = 0
            //Array > 0
        };

        union Data {
            void* ptr;
            char c;
            double f;
            long i;
            bool b;
            std::reference_wrapper<Object> *a;
        } data;
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

    Environment get_environment(std::shared_ptr<Expression> tree);

    void execute(std::shared_ptr<Expression> tree, Environment & environment, std::vector<Object> & objects);

}


#endif
