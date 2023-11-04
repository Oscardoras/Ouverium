#ifndef __COMPILER_C_CODE_HPP__
#define __COMPILER_C_CODE_HPP__

#include <list>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "../Analyzer.hpp"


namespace Translator::CStandard {

    struct Type {

        std::string name;

        virtual ~Type() = default;

    };
    using Declarations = std::map<std::string, std::weak_ptr<Type>>;
    struct Class: public Type {
        Declarations properties;
    };
    inline std::shared_ptr<Type> Unknown = nullptr;
    inline std::shared_ptr<Type> Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> Float = std::make_shared<Type>();


    struct Expression {};
    struct LValue: public Expression {};
    struct Instruction {};
    using Instructions = std::list<std::shared_ptr<Instruction>>;

    struct Reference: public LValue {
        bool owned;
        //std::weak_ptr<Type> type;
        Reference(bool owned) :
            owned{ owned } {}
    };

    struct If: public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        Instructions alternative;
        If(std::shared_ptr<Expression> condition, Instructions const& body, Instructions const& alternative) :
            condition{ condition }, body{ body }, alternative{ alternative } {}
    };

    struct While: public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        While(std::shared_ptr<Expression> condition, Instructions const& body) :
            condition{ condition }, body{ body } {}
    };

    struct Affectation: public Expression, public Instruction {
        std::shared_ptr<LValue> lvalue;
        std::shared_ptr<Expression> value;
        Affectation(std::shared_ptr<LValue> lvalue, std::shared_ptr<Expression> value) :
            lvalue{ lvalue }, value{ value } {}
    };

    struct Symbol: public LValue {
        std::string name;
        Symbol(std::string const& name) :
            name{ name } {}
    };

    struct Value: public Expression {
        std::variant<char, bool, long, double, std::string> value;
        Value(std::variant<char, bool, long, double, std::string> const& value) :
            value{ value } {}
    };

    struct FunctionCall: public Expression, public Instruction {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> parameters;
        FunctionCall(std::shared_ptr<Expression> function, std::vector<std::shared_ptr<Expression>> const& parameters) :
            function{ function }, parameters{ parameters } {}
    };

    struct Property: public LValue {
        std::shared_ptr<Expression> object;
        std::string name;
        bool pointer;
        Property(std::shared_ptr<Expression> object, std::string const& name, bool pointer) :
            object{ object }, name{ name }, pointer{ pointer } {}
    };

    struct List: public Expression {
        std::vector<std::shared_ptr<Expression>> objects;
        List(std::vector<std::shared_ptr<Expression>> const& objects) :
            objects{ objects } {}
    };

/*
    struct Array: public Instruction {
        std::weak_ptr<Type> type;
        std::string name;
        std::shared_ptr<List> list;
        Array(std::weak_ptr<Type> type, std::string const&name, std::shared_ptr<List> list) :
            type{ type }, name{ name }, list{ list } {}
    };

    struct Object: public Instruction {
        std::string type;
        std::string name;
        std::shared_ptr<List> list;
        Object(std::string type, std::string const&name, std::shared_ptr<List> list) :
            type{ type }, name{ name }, list{ list } {}
    };
*/

    struct FunctionExpression: public Expression, std::variant<std::shared_ptr<FunctionExpression>, std::shared_ptr<Reference>> {
        using std::variant<std::shared_ptr<FunctionExpression>, std::shared_ptr<Reference>>::variant;
    };


    struct FunctionDefinition: public Expression {
        using Parameter = std::string;
        using Parameters = std::vector<Parameter>;
        //std::weak_ptr<Type> type;
        std::string name;
        Parameters parameters;
        std::string format;
        Declarations local_variables;
        Instructions body;
        std::shared_ptr<Reference> return_value;
    };

}


#endif
