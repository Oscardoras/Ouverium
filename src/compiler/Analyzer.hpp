#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <list>
#include <variant>

#include "../interpreter/Interpreter.hpp"

#include "../parser/expression/Expression.hpp"
#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"


namespace Analyzer {

    struct Symbol {
        std::string name;
        bool reference;
    };

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
    struct Type {
        bool pointer;
        std::map<std::string, std::shared_ptr<Type>> properties;
        Object::ObjectType type;
    };
    struct MetaData {
        std::map<std::shared_ptr<Expression>, std::shared_ptr<Type>> types;
        std::map<std::shared_ptr<FunctionCall>, std::vector<FunctionPointer>> links;
    };

}


#endif
