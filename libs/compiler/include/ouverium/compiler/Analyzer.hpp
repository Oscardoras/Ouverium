#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include <ouverium/parser/Expressions.hpp>


namespace Analyzer {

    struct Type {
        std::string name;
        virtual ~Type() = default;
    };
    using Types = std::set<std::weak_ptr<Type>>;

    inline std::shared_ptr<Type> const Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> const Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> const Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> const Float = std::make_shared<Type>();

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
        std::map<std::shared_ptr<Parser::FunctionCall>, std::shared_ptr<Parser::Expression>> sources;
    };

    inline bool operator<(std::weak_ptr<Type> a, std::weak_ptr<Type> b) {
        return a.lock() < b.lock();
    }

    class Exception {

    public:

        std::vector<Parser::Position> positions;

    };

}
