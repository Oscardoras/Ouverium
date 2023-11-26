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

        Type(std::string const& name):
            name{ name } {}

        virtual ~Type() = default;

    };
    using Declarations = std::map<std::string, std::weak_ptr<Type>>;
    struct Structure: public Type {
        Declarations properties;
        std::weak_ptr<Type> array;
        bool function = false;

        using Type::Type;
    };
    inline std::shared_ptr<Type> Unknown = std::make_shared<Type>("UnknownData");
    inline std::shared_ptr<Type> Bool = std::make_shared<Type>("Bool");
    inline std::shared_ptr<Type> Char = std::make_shared<Type>("Char");
    inline std::shared_ptr<Type> Int = std::make_shared<Type>("Int");
    inline std::shared_ptr<Type> Float = std::make_shared<Type>("Float");


    struct Expression {
        virtual std::string get_expression_code() const = 0;
    };
    struct LValue: public Expression {};
    struct Instruction {
        virtual std::string get_instruction_code() const = 0;
    };
    using Instructions = std::list<std::shared_ptr<Instruction>>;

    struct Declaration: public Instruction {
        std::string type;
        std::shared_ptr<Symbol> symbol;
        Declaration(std::string const& type, std::shared_ptr<LValue> lvalue):
            type{ type }, symbol{ symbol } {}

        virtual std::string get_instruction_code() const;
    };

    struct Reference: public LValue, public Instruction {
        unsigned number;
        bool owned;
        //std::weak_ptr<Type> type;
        Reference(bool owned) :
            owned{ owned } {
            number = count++;
        }

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    private:
        inline static unsigned count = 0;
    };

    struct If: public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        Instructions alternative;
        If(std::shared_ptr<Expression> condition, Instructions const& body, Instructions const& alternative) :
            condition{ condition }, body{ body }, alternative{ alternative } {}

        std::string get_instruction_code() const override;
    };

    struct While: public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        While(std::shared_ptr<Expression> condition, Instructions const& body) :
            condition{ condition }, body{ body } {}

        std::string get_instruction_code() const override;
    };

    struct Affectation: public Expression, public Instruction {
        std::shared_ptr<LValue> lvalue;
        std::shared_ptr<Expression> value;
        Affectation(std::shared_ptr<LValue> lvalue, std::shared_ptr<Expression> value) :
            lvalue{ lvalue }, value{ value } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };

    struct Symbol: public LValue {
        std::string name;
        Symbol(std::string const& name) :
            name{ name } {}

        std::string get_expression_code() const override;
    };

    struct Value: public Expression {
        std::variant<char, bool, long, double, std::string> value;
        Value(std::variant<char, bool, long, double, std::string> const& value) :
            value{ value } {}

        std::string get_expression_code() const override;

    private:
        struct Visitor {
            std::string operator()(auto v) {
                return std::to_string(v);
            }
            std::string operator()(std::string const& s) {
                return "\"" + s + "\"";
            }
        };
    };

    struct FunctionCall: public Expression, public Instruction {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> parameters;
        FunctionCall(std::shared_ptr<Expression> function, std::vector<std::shared_ptr<Expression>> const& parameters) :
            function{ function }, parameters{ parameters } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };

    struct Referencing: public Expression {
        std::shared_ptr<Expression> expression;
        Referencing(std::shared_ptr<Expression> expression) :
            expression{ expression } {}

        std::string get_expression_code() const override;
    };

    struct Property: public LValue {
        std::shared_ptr<Expression> object;
        std::string name;
        bool pointer;
        Property(std::shared_ptr<Expression> object, std::string const& name, bool pointer) :
            object{ object }, name{ name }, pointer{ pointer } {}

        std::string get_expression_code() const override;
    };

    struct List: public Expression, public Instruction {
        std::vector<std::shared_ptr<Reference>> objects;
        List(std::vector<std::shared_ptr<Reference>> const& objects = {}) :
            objects{ objects } {
            number = count++;
        }

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    private:
        inline static unsigned count = 0;
        unsigned number;
    };

    struct FunctionExpression: public Expression, public Instruction, std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>> {
        FunctionExpression(std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>> const& variant) :
            std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>>{ variant } {
            number = count++;
        }

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    private:
        inline static unsigned count = 0;
        unsigned number;
    };


    struct FunctionDefinition: public Expression {
        using Parameter = Symbol;
        using Parameters = std::vector<Parameter>;
        std::string name;
        std::vector<Symbol> captures;
        Parameters parameters;
        std::string format;
        struct {
            Declarations local_variables;
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } body;
        struct {
            Declarations local_variables;
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } filter;

        std::string get_expression_code() const override;
    };

}


#endif
