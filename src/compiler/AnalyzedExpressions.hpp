#ifndef __COMPILER_ANALYZEDEXPRESSIONS_HPP__
#define __COMPILER_ANALYZEDEXPRESSIONS_HPP__

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <variant>


namespace Analyzer {

    /**
     * Represents an analyzed expression, must be inherited.
    */
    struct AnalyzedExpression {

        virtual ~AnalyzedExpression() = default;

    protected:

        AnalyzedExpression() = default;

    };

    struct Type {};
    inline std::shared_ptr<Type> Pointer = std::make_shared<Type>();
    inline std::shared_ptr<Type> Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> Float = std::make_shared<Type>();
    struct Structure: public Type {

        std::map<std::string, std::set<std::weak_ptr<Type>>> properties;

        Structure(std::map<std::string, std::set<std::weak_ptr<Type>> properties = {}):
            properties(properties) {}

    };

    struct FunctionDefinition {

        std::set<std::weak_ptr<Type>> return_type;
        std::shared_ptr<AnalyzedExpression> parameters;
        std::shared_ptr<AnalyzedExpression> filter;
        std::shared_ptr<AnalyzedExpression> body;

        FunctionDefinition(std::set<std::weak_ptr<Type>> const& return_type = {}, std::shared_ptr<AnalyzedExpression> parameters = nullptr, std::shared_ptr<AnalyzedExpression> filter = nullptr, std::shared_ptr<AnalyzedExpression> body = nullptr):
            return_type(return_type), parameters(parameters), filter(filter), body(body) {}

    };


    struct FunctionCall: public AnalyzedExpression {

        std::shared_ptr<AnalyzedExpression> function;
        std::shared_ptr<AnalyzedExpression> arguments;

        FunctionCall(std::shared_ptr<AnalyzedExpression> function = nullptr, std::shared_ptr<AnalyzedExpression> arguments = nullptr) {}

    };
    struct FunctionRun: public AnalyzedExpression {

        FunctionDefinition& function;
        std::shared_ptr<AnalyzedExpression> arguments;

        FunctionRun(FunctionDefinition & function, std::shared_ptr<AnalyzedExpression> arguments = nullptr):
            function(function), arguments(arguments) {}

    };

    struct Property: public AnalyzedExpression {

        std::shared_ptr<AnalyzedExpression> object;
        std::string name;

        Property(std::shared_ptr<AnalyzedExpression> object = nullptr, std::string const& name = ""):
            object(object), name(name) {}

    };

    struct Tuple: public AnalyzedExpression {

        std::vector<std::shared_ptr<AnalyzedExpression>> objects;

        Tuple(std::initializer_list<std::shared_ptr<AnalyzedExpression>> const& l = {}):
            objects(l) {}

    };

    struct Symbol: public AnalyzedExpression {

        std::string name;

        Symbol(std::string const& name):
            name(name) {}

    };

    struct Value: public AnalyzedExpression {

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
        std::map<std::string, std::shared_ptr<Structure>> structures;
        std::map<std::string, std::shared_ptr<FunctionDefinition>> function_definitions;
    };

}


#endif
