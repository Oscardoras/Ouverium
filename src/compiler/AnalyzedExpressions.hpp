#ifndef __COMPILER_ANALYZEDEXPRESSIONS_HPP__
#define __COMPILER_ANALYZEDEXPRESSIONS_HPP__

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace Analyzer {

    /**
     * Represents an analyzed expression, must be inherited.
    */
    struct AnalyzedExpression {

        virtual ~AnalyzedExpression() = default;

    protected:

        AnalyzedExpression() = default;

    };

    struct Type: public AnalyzedExpression {

        std::string name;

        Type(std::string const& name = ""):
            name(name) {}

    };
    inline static Type Pointer = Type("");
    inline static Type Bool = Type("Bool");
    inline static Type Char = Type("Char");
    inline static Type Int = Type("Int");
    inline static Type Float = Type("Float");
    struct Structure: public Type {

        std::map<std::string, std::set<Type&>> properties;

        Structure(std::map<std::string, std::set<Type&>> properties = {}):
            properties(properties) {}

    };

    struct Function: public AnalyzedExpression {

        std::set<Type&> return_type;
        std::string name;
        std::shared_ptr<AnalyzedExpression> parameters;
        std::shared_ptr<AnalyzedExpression> filter;
        std::shared_ptr<AnalyzedExpression> body;

        Function(std::set<Type&> return_type, std::string name = "", std::shared_ptr<AnalyzedExpression> parameters = nullptr, std::shared_ptr<AnalyzedExpression> filter = nullptr, std::shared_ptr<AnalyzedExpression> body = nullptr):
            return_type(return_type), name(name), parameters(parameters), filter(filter), body(body) {}

    };
    struct FunctionCall: public AnalyzedExpression {

        std::shared_ptr<AnalyzedExpression> function;
        std::shared_ptr<AnalyzedExpression> arguments;

        FunctionCall(std::shared_ptr<AnalyzedExpression> function = nullptr, std::shared_ptr<AnalyzedExpression> arguments = nullptr) {}

    };
    struct FunctionRun: public AnalyzedExpression {

        Function& function;
        std::shared_ptr<AnalyzedExpression> arguments;

        FunctionRun(Function & function, std::shared_ptr<AnalyzedExpression> arguments = nullptr):
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

}


#endif
