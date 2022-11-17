#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "Structures.hpp"

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

    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, Reference (*)(FunctionContext&)>;
    using Links = std::map<std::shared_ptr<FunctionCall>, std::vector<FunctionPointer>>;
    struct Type {
        bool pointer;
        std::map<std::string, std::shared_ptr<Type>> properties;
        Object::ObjectType type;
    };
    using Types = std::map<std::shared_ptr<Expression>, std::shared_ptr<Type>>;

    GlobalEnvironment get_environment(std::shared_ptr<Expression> tree);

    void execute(std::shared_ptr<Expression> tree);

    std::vector<std::shared_ptr<CStructures::Instruction>> get_instructions(std::shared_ptr<Expression> expression, Types & types, Links & links);
    std::shared_ptr<CStructures::Expression> get_expression(std::shared_ptr<Expression> expression, Types & types, Links & links);

}


#endif
