#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <filesystem>
#include <list>
#include <variant>

#include "Code.hpp"

#include "../Analyzer.hpp"


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
            struct {
                Declarations global_variables;
                Instructions body;
                std::shared_ptr<Reference> return_value;
            } main;
            std::set<std::shared_ptr<FunctionDefinition>> functions;
            std::set<std::shared_ptr<Lambda>> lambdas;
        } code;

        void create_structures();
        std::shared_ptr<FunctionDefinition> create_function(std::shared_ptr<Parser::FunctionDefinition> function);

        std::shared_ptr<Reference> eval_system_function(Analyzer::SystemFunction const& function, std::shared_ptr<Parser::Expression> arguments, Instructions& instructions, Instructions::iterator it);
        std::shared_ptr<Reference> get_expression(std::shared_ptr<Parser::Expression> expression, Instructions& instructions, Instructions::iterator it);

        void write_structures(std::string& interface, std::string& implementation);
        void write_functions(std::string& interface, std::string& implementation);
        void write_main(std::string& implementation);

    public:

        Translator(std::shared_ptr<Parser::Expression> expression, Analyzer::MetaData const& meta_data) :
            expression{ expression }, meta_data{ meta_data } {}

        void translate(std::filesystem::path const& out);

        inline static const std::set<std::string> symbols = {
            "getter",
            "defined",
            "setter",
            "_x3A_x3D",
            "_x3B",
            "if_statement",
            "else_statement",
            "while_statement",
            "_x24",
            "_x24_x3D_x3D",
            "_x3A",
            "_x3D_x3D",
            "_x21_x3D",
            "_x3D_x3D_x3D",
            "_x21_x3D_x3D",
            "string_from",
            "print",
            "_x21",
            "_x26",
            "_x7C",
            "_x2B",
            "_x2D",
            "_x2A",
            "_x2F",
            "_x25",
        };

    };

}


#endif
