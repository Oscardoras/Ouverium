#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "../../interpreter/Interpreter.hpp"

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

/*
    struct Object {
        std::shared_ptr<Expression> creation;
        bool is_stack;


    };
*/

    struct FunctionEnvironment {
        std::shared_ptr<FunctionDefinition> expression;
        std::vector<Symbol> parameters;
        std::vector<Symbol> local_variables;
    };

    struct GlobalEnvironment {
        std::vector<Symbol> global_variables;
        std::vector<FunctionEnvironment> functions;
    };

    struct CompilerContext: public GlobalContext {

        std::vector<Object*> incomplete;

        CompilerContext();

        ~CompilerContext();

    };

    struct Execution {
        Reference reference;
        bool complete;
    };

    GlobalEnvironment get_environment(std::shared_ptr<Expression> tree);

    void execute(std::shared_ptr<Expression> tree);

}


#endif
