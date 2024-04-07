#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>

#include "../parser/Expressions.hpp"


namespace Analyzer {

    struct Type {
        std::string name;
        virtual ~Type() = default;
    };
    using Types = std::set<std::weak_ptr<Type>>;

    inline std::shared_ptr<Type> Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> Float = std::make_shared<Type>();

    struct Structure : public Type {
        std::map<std::string, Types> properties;
        Types array;
        bool function = false;
    };


    using CustomFunction = std::shared_ptr<Parser::FunctionDefinition>;
    using SystemFunction = std::string;
    using Function = std::variant<CustomFunction, SystemFunction>;


    struct MetaData {
        std::set<std::shared_ptr<Structure>> structures;
        std::map<std::shared_ptr<Parser::Expression>, Types> types;
        std::map<std::shared_ptr<Parser::FunctionCall>, std::set<Function>> calls;
    };

    inline bool operator<(std::weak_ptr<Type> a, std::weak_ptr<Type> b) {
        return a.lock() < b.lock();
    }

}
