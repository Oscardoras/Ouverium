#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <list>
#include <variant>

#include "../interpreter/Interpreter.hpp"

#include "../Expressions.hpp"


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

    struct CompilerContext: public Interpreter::GlobalContext {

        std::vector<Interpreter::Object*> incomplete;

        CompilerContext();

        ~CompilerContext();

    };

    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, Interpreter::Reference (*)(Interpreter::FunctionContext&)>;
    struct Type {
        bool pointer;
        std::map<std::string, std::shared_ptr<Type>> properties;
        Interpreter::Object::ObjectType type;
    };
    struct MetaData {
        std::map<std::shared_ptr<Expression>, std::shared_ptr<Type>> types;
        std::map<std::shared_ptr<FunctionCall>, std::vector<FunctionPointer>> links;
    };

}


#endif
