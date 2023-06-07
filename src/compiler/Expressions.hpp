#ifndef __COMPILER_EXPRESSIONS_HPP__
#define __COMPILER_EXPRESSIONS_HPP__

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <variant>


namespace Analyzer {

    struct Type {
        virtual ~Type() = default;
    };
    inline std::shared_ptr<Type> Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> Float = std::make_shared<Type>();
    struct Structure: public Type {

        std::map<std::string, std::set<std::weak_ptr<Type>>> properties;

        Structure(std::map<std::string, std::set<std::weak_ptr<Type>>> properties = {}):
            properties(properties) {}

    };

    using Types = std::set<std::weak_ptr<Type>>;

    /**
     * Represents an analyzed expression, must be inherited.
    */
    struct Expression {

        Types types;

        virtual ~Expression() = default;

    };

    struct FunctionDefinition {

        std::set<std::weak_ptr<Type>> return_type;
        std::shared_ptr<Expression> parameters;
        std::map<std::string, Types> local_variables;
        std::shared_ptr<Expression> filter;
        std::shared_ptr<Expression> body;

        FunctionDefinition(std::set<std::weak_ptr<Type>> const& return_type = {}, std::shared_ptr<Expression> parameters = nullptr, std::shared_ptr<Expression> filter = nullptr, std::shared_ptr<Expression> body = nullptr):
            return_type(return_type), parameters(parameters), filter(filter), body(body) {}

    };

    using SystemFunction = std::string;

    struct FunctionCall: public Expression {

        std::shared_ptr<Expression> function;
        std::shared_ptr<Expression> arguments;

        FunctionCall(std::shared_ptr<Expression> function = nullptr, std::shared_ptr<Expression> arguments = nullptr) {}

    };
    struct FunctionRun: public Expression {

        std::variant<std::weak_ptr<FunctionDefinition>, SystemFunction> function;
        std::shared_ptr<Expression> arguments;

        FunctionRun(std::weak_ptr<FunctionDefinition> function, std::shared_ptr<Expression> arguments = nullptr):
            function(function), arguments(arguments) {}

    };

    struct Property: public Expression {

        std::shared_ptr<Expression> object;
        std::string name;

        Property(std::shared_ptr<Expression> object = nullptr, std::string const& name = ""):
            object(object), name(name) {}

    };

    struct Tuple: public Expression {

        std::vector<std::shared_ptr<Expression>> objects;

        Tuple(std::initializer_list<std::shared_ptr<Expression>> const& l = {}):
            objects(l) {}

    };

    struct Symbol: public Expression {

        std::string name;

        Symbol(std::string const& name):
            name(name) {}

    };

    struct Value: public Expression {

        std::variant<char, bool, long, double, std::string> value;

        Value(std::variant<char, bool, long, double, std::string> const& value):
            value(value) {}

        Value(std::variant<nullptr_t, bool, long, double, std::string> const& symbol) {
            if (auto b = std::get_if<bool>(&symbol))
                value = *b;
            else if (auto l = std::get_if<long>(&symbol))
                value = *l;
            else if (auto d = std::get_if<double>(&symbol))
                value = *d;
            else if (auto str = std::get_if<std::string>(&symbol))
                value = *str;
        }

    };


    struct MetaData {
        std::set<std::shared_ptr<Structure>> structures;
        std::set<std::shared_ptr<FunctionDefinition>> function_definitions;
        std::map<std::string, Types> global_variables;
    };

}


#endif