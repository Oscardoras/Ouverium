#ifndef __COMPILER_C_CODE_HPP__
#define __COMPILER_C_CODE_HPP__

#include <list>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "../Analyzer.hpp"

#include "../../Types.hpp"


namespace Translator::CStandard {

    struct Name {
        std::string symbol;

        Name(std::string const& symbol) :
            symbol{ symbol } {}
        Name(const char* symbol) :
            symbol{ symbol } {}

        std::string get() const;
        operator std::string() const {
            return get();
        }

        friend auto operator<=>(Name const& a, Name const& b) {
            return a.symbol <=> b.symbol;
        }
    };

    struct Type {

        Name name;

        Type(Name const& name) :
            name{ name } {}

        virtual ~Type() = default;

    };
    using Declarations = std::map<Name, std::weak_ptr<Type>>;
    struct Structure : public Type {
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
    struct LValue : public Expression {};
    struct Instruction {
        virtual std::string get_instruction_code() const = 0;
    };
    using Instructions = std::list<std::shared_ptr<Instruction>>;

    struct If : public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        Instructions alternative;
        If(std::shared_ptr<Expression> condition, Instructions const& body, Instructions const& alternative) :
            condition{ condition }, body{ body }, alternative{ alternative } {}

        std::string get_instruction_code() const override;
    };

    struct While : public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        While(std::shared_ptr<Expression> condition, Instructions const& body) :
            condition{ condition }, body{ body } {}

        std::string get_instruction_code() const override;
    };

    struct Symbol : public LValue {
        Name name;
        Symbol(Name const& name) :
            name{ name } {}

        std::string get_expression_code() const override;
    };

    struct Declaration : public Instruction {
        std::string type;
        std::shared_ptr<Symbol> symbol;
        Declaration(std::string const& type, std::shared_ptr<Symbol> symbol) :
            type{ type }, symbol{ symbol } {}

        virtual std::string get_instruction_code() const;
    };

    struct Affectation : public Expression, public Instruction {
        std::shared_ptr<LValue> lvalue;
        std::shared_ptr<Expression> value;
        Affectation(std::shared_ptr<LValue> lvalue, std::shared_ptr<Expression> value) :
            lvalue{ lvalue }, value{ value } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };

    struct Value : public Expression {
        std::variant<char, bool, OV_INT, OV_FLOAT, std::string> value;
        Value(std::variant<char, bool, OV_INT, OV_FLOAT, std::string> const& value) :
            value{ value } {}

        std::string get_expression_code() const override;

    private:
        struct Visitor {
            std::string operator()(auto v) {
                return std::to_string(v);
            }
            std::string operator()(std::string const& s) {
                return s;
            }
        };
    };

    struct FunctionCall : public Expression, public Instruction {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> parameters;
        FunctionCall(std::shared_ptr<Expression> function, std::vector<std::shared_ptr<Expression>> const& parameters) :
            function{ function }, parameters{ parameters } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };

    struct Referencing : public Expression {
        std::shared_ptr<Expression> expression;
        Referencing(std::shared_ptr<Expression> expression) :
            expression{ expression } {}

        std::string get_expression_code() const override;
    };

    struct Property : public LValue {
        std::shared_ptr<Expression> object;
        Name name;
        bool pointer;
        Property(std::shared_ptr<Expression> object, Name const& name, bool pointer) :
            object{ object }, name{ name }, pointer{ pointer } {}

        std::string get_expression_code() const override;
    };

    class Id {
        inline static unsigned count = 0;
    protected:
        unsigned id;
        Id() {
            id = count++;
        }
    public:
        unsigned get_id() const {
            return id;
        }
    };

    struct Reference : public LValue, public Instruction, public Id {
        bool owned;
        std::weak_ptr<Type> type;
        Reference(bool owned) :
            owned{ owned } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };

    struct List : public Expression, public Instruction, public Id {
        std::vector<std::shared_ptr<Expression>> objects;
        List(std::vector<std::shared_ptr<Expression>> const& objects = {}) :
            objects{ objects } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };

    struct Lambda : public Id {
        Name name;
        std::vector<std::pair<Name, std::weak_ptr<Type>>> captures;
        struct {
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } body;

        Lambda() :
            name{ "lambda_" + std::to_string(id) } {}
    };

    struct FunctionExpression : public Expression, public Instruction, std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>, std::shared_ptr<Lambda>>, public Id {
        FunctionExpression(std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>, std::shared_ptr<Lambda>> const& variant) :
            std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>, std::shared_ptr<Lambda>>{ variant } {}

        std::string get_expression_code() const override;
        std::string get_instruction_code() const override;
    };


    struct FunctionDefinition : public Id {
        Name name;
        std::vector<std::pair<Name, std::weak_ptr<Type>>> captures;
        std::vector<std::pair<Name, std::weak_ptr<Type>>> parameters;
        std::string format;
        Declarations local_variables;
        struct {
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } body;
        struct {
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } filter;

        FunctionDefinition() :
            name{ "function_" + std::to_string(id) } {}
    };


    struct Code {
        std::set<std::shared_ptr<Structure>> structures;
        std::set<std::shared_ptr<FunctionDefinition>> functions;
        struct {
            Declarations global_variables;
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } main;
    };

}


#endif
