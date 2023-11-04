#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>

#include "../parser/Parser.hpp"


namespace Analyzer {

    struct Type {
        virtual ~Type() = default;
    };
    using Types = std::vector<std::weak_ptr<Type>>;

    inline std::shared_ptr<Type> Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> Float = std::make_shared<Type>();

    struct Structure: public Type {
        std::map<std::string, std::set<std::weak_ptr<Type>>> properties;
        std::set<std::weak_ptr<Type>> array;
        bool function = false;
    };


    using CustomFunction = std::shared_ptr<Parser::FunctionDefinition>;
    using SystemFunction = std::string;
    using Function = std::variant<CustomFunction, SystemFunction>;


    struct MetaData {
        std::set<std::shared_ptr<Structure>> structures;
        std::map<std::shared_ptr<Parser::Expression>, std::set<std::shared_ptr<Type>>> types;
        std::map<std::shared_ptr<Parser::FunctionCall>, std::set<Function>> calls;
    };

}
