#pragma once

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <ouverium/types.h>


namespace Translator::CStandard {

    struct Name {
        std::string symbol;

        Name(std::string symbol = {}) :
            symbol{ std::move(symbol) } {}
        Name(const char* symbol) :
            symbol{ symbol } {}

        [[nodiscard]] std::string get() const;

        friend auto operator==(Name const& a, Name const& b) {
            return a.symbol == b.symbol;
        }

        friend auto operator<=>(Name const& a, Name const& b) {
            return a.symbol <=> b.symbol;
        }
    };

    struct Type {
        Name name;

        Type(Name name) :
            name{ std::move(name) } {}

        virtual ~Type() = default;
    };
    using Declarations = std::map<Name, std::weak_ptr<Type>>;
    struct Structure : public Type {
        Declarations properties;
        std::weak_ptr<Type> array;
        bool function = false;

        using Type::Type;
    };
    inline std::shared_ptr<Type> const Unknown = std::make_shared<Type>("UnknownData");
    inline std::shared_ptr<Type> const Bool = std::make_shared<Type>("Bool");
    inline std::shared_ptr<Type> const Char = std::make_shared<Type>("Char");
    inline std::shared_ptr<Type> const Int = std::make_shared<Type>("Int");
    inline std::shared_ptr<Type> const Float = std::make_shared<Type>("Float");


    struct Expression {
        virtual ~Expression() = default;
        [[nodiscard]] virtual std::string get_expression_code() const = 0;
    };
    struct LValue : public Expression {};
    struct Instruction {
        virtual ~Instruction() = default;
        [[nodiscard]] virtual std::string get_instruction_code() const = 0;
    };
    using Instructions = std::list<std::shared_ptr<Instruction>>;

    struct If : public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;
        Instructions alternative;

        If(std::shared_ptr<Expression> condition, Instructions body = {}, Instructions alternative = {}) :
            condition{ condition }, body{ std::move(body) }, alternative{ std::move(alternative) } {}

        [[nodiscard]] std::string get_instruction_code() const override;
    };

    struct While : public Instruction {
        std::shared_ptr<Expression> condition;
        Instructions body;

        While(std::shared_ptr<Expression> condition, Instructions body) :
            condition{ condition }, body{ std::move(body) } {}

        [[nodiscard]] std::string get_instruction_code() const override;
    };

    struct Symbol : public LValue {
        Name name;

        Symbol(Name name) :
            name{ std::move(name) } {}

        [[nodiscard]] std::string get_expression_code() const override;
    };

    struct Declaration : public Instruction {
        std::string type;
        std::shared_ptr<Symbol> symbol;

        Declaration(std::string type, std::shared_ptr<Symbol> symbol) :
            type{ std::move(type) }, symbol{ symbol } {}

        [[nodiscard]] std::string get_instruction_code() const override;
    };

    struct Value : public Expression {
        std::variant<std::nullptr_t, char, bool, OV_INT, OV_FLOAT, std::string> value;

        Value(std::variant<std::nullptr_t, char, bool, OV_INT, OV_FLOAT, std::string> const& value) :
            value{ value } {}

        [[nodiscard]] std::string get_expression_code() const override;

    private:
        struct Visitor {
            std::string operator()(std::nullptr_t /*v*/) {
                return "NULL";
            }
            std::string operator()(auto v) {
                return std::to_string(v);
            }
            std::string operator()(std::string const& s) {
                return "\"" + s + "\"";
            }
        };
    };

    struct FunctionCall : public Expression, public Instruction {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> parameters;

        FunctionCall(std::shared_ptr<Expression> function, std::vector<std::shared_ptr<Expression>> parameters) :
            function{ function }, parameters{ std::move(parameters) } {}

        [[nodiscard]] std::string get_expression_code() const override;
        [[nodiscard]] std::string get_instruction_code() const override;
    };

    struct Referencing : public Expression {
        std::shared_ptr<Expression> expression;

        Referencing(std::shared_ptr<Expression> expression) :
            expression{ expression } {}

        [[nodiscard]] std::string get_expression_code() const override;
    };

    struct Property : public LValue {
        std::shared_ptr<Expression> object;
        Name name;
        bool pointer;

        Property(std::shared_ptr<Expression> object, Name name, bool pointer) :
            object{ object }, name{ std::move(name) }, pointer{ pointer } {}

        [[nodiscard]] std::string get_expression_code() const override;
    };

    class Id {
        inline static unsigned count = 0;
    protected:
        unsigned id;
        Id() :
            id(count++) {}
    public:
        [[nodiscard]] unsigned get_id() const {
            return id;
        }
    };

    struct Reference : public LValue, public Id {
        bool owned;
        std::weak_ptr<Type> type;
        std::string name;

        Reference(bool owned) :
            owned{ owned } {
            name = "reference" + std::to_string(id);
        }
        Reference(bool owned, Name const& name) :
            owned{ owned }, name{ name.get() } {}

        [[nodiscard]] std::string get_expression_code() const override;
    };

    struct RawData : public Expression {
        std::shared_ptr<Value> value;

        RawData(std::shared_ptr<Value> value) :
            value{ value } {}

        [[nodiscard]] std::string get_expression_code() const override;
    };

    struct List : public Expression, public Instruction, public Id {
        std::vector<std::shared_ptr<Expression>> objects;

        List(std::vector<std::shared_ptr<Expression>> const& objects = {}) :
            objects{ objects } {}

        [[nodiscard]] std::string get_expression_code() const override;
        [[nodiscard]] std::string get_instruction_code() const override;
    };

    struct Affectation : public Expression, public Instruction {
        bool declare;
        std::shared_ptr<LValue> lvalue;
        std::shared_ptr<Expression> value;

        Affectation(std::shared_ptr<LValue> lvalue, std::shared_ptr<Expression> value, bool declare = true) :
            declare{ declare }, lvalue{ lvalue }, value{ value } {}

        [[nodiscard]] std::string get_expression_code() const override;
        [[nodiscard]] std::string get_instruction_code() const override;
    };

    struct Lambda : public Id {
        std::vector<std::pair<Name, std::weak_ptr<Type>>> captures;
        struct {
            Instructions body;
            std::shared_ptr<Reference> return_value;
        } body;
    };

    struct FunctionExpression : public Expression, public Instruction, std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>, std::shared_ptr<Lambda>>, public Id {
        FunctionExpression(std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>, std::shared_ptr<Lambda>> const& variant) :
            std::variant<std::vector<std::shared_ptr<FunctionExpression>>, std::shared_ptr<Reference>, std::shared_ptr<Lambda>>{ variant } {}

        [[nodiscard]] std::string get_expression_code() const override;
        [[nodiscard]] std::string get_instruction_code() const override;
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

        void set_name(std::string const& str) {
            name = str + "_" + std::to_string(id);
        }
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
