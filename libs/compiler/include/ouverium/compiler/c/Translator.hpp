#pragma once

#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include <ouverium/compiler/c/Code.hpp>
#include <ouverium/compiler/Analyzer.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Translator::CStandard {

    struct TypeTable : public std::map<std::shared_ptr<Analyzer::Type>, std::shared_ptr<Type>> {

        TypeTable() {
            operator[](Analyzer::Bool) = Bool;
            operator[](Analyzer::Char) = Char;
            operator[](Analyzer::Int) = Int;
            operator[](Analyzer::Float) = Float;
        }

        std::shared_ptr<Type> get(Analyzer::Types const& types) {
            if (types.size() == 1)
                return operator[](types.begin()->lock());
            else
                return Unknown;
        }

    };
    using FunctionTable = std::map<std::shared_ptr<Parser::FunctionDefinition>, std::shared_ptr<FunctionDefinition>>;

    class Translator {

        std::shared_ptr<Parser::Expression> expression;
        Analyzer::MetaData meta_data;

        TypeTable type_table;
        FunctionTable function_table;

        struct {
            std::set<std::shared_ptr<Structure>> structures;
            Declarations global_variables;
            struct Main {
                Instructions body;
                std::shared_ptr<Reference> return_value;
            } main;
            std::set<std::shared_ptr<FunctionDefinition>> functions;
            std::set<std::shared_ptr<Lambda>> lambdas;
            std::map<std::string, Main> imports;
        } code;

        void create_structures();
        std::shared_ptr<FunctionDefinition> create_function(std::shared_ptr<Parser::FunctionDefinition> function);

        std::shared_ptr<Reference> eval_system_function(Analyzer::SystemFunction const& function, std::shared_ptr<Parser::Expression> arguments, Instructions& instructions, Instructions::iterator it);
        std::shared_ptr<Reference> get_expression(std::shared_ptr<Parser::Expression> expression, Instructions& instructions, Instructions::iterator it);

        void write_structures(std::string& interface, std::string& implementation);
        void write_global_variables(std::string& interface, std::string& implementation);
        void write_functions(std::string& interface, std::string& implementation);
        void write_main(std::string& implementation);

    public:

        Translator(std::shared_ptr<Parser::Expression> expression, Analyzer::MetaData meta_data) :
            expression{ expression }, meta_data{ std::move(meta_data) } {}

        void translate(std::filesystem::path const& out);

        inline static const std::set<std::string> symbols = {
            "getter",
            "defined",
            "setter",
            ":=",
            ";",
            "if",
            "else",
            "while",
            "for",
            "from",
            "to",
            "step",
            "$",
            "$==",
            ":",
            "==",
            "!=",
            "===",
            "!==",
            "string_from",
            "print",
            "scan",
            "Char",
            "Float",
            "Int",
            "Bool",
            "Array",
            "Function",
            "foreach",
            "~",
            "!",
            "&",
            "|",
            "+",
            "-",
            "*",
            "/",
            "%",
        };

    };

}
