#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <filesystem>
#include <list>
#include <variant>

#include "Code.hpp"

#include "../Analyzer.hpp"


namespace Translator::CStandard {

    struct TypeTable: public std::map<std::shared_ptr<Analyzer::Type>, std::shared_ptr<Type>> {

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

        std::shared_ptr<Expression> get_unknown_data(std::shared_ptr<Expression> expression);

        struct {
            Instructions main_instructions;
            std::set<std::shared_ptr<Structure>> structures;
            std::set<std::shared_ptr<FunctionDefinition>> functions;
            std::shared_ptr<Expression> main;
        } code;

        void create_structures();
        std::shared_ptr<FunctionDefinition> create_function(std::shared_ptr<Parser::FunctionDefinition> function);

        void add_system_function(std::string symbol, std::string function, Instructions & instructions, Instructions::iterator it);
        std::shared_ptr<Reference> eval_system_function(Analyzer::SystemFunction const& function, std::shared_ptr<Parser::Expression> arguments, Instructions & instructions, Instructions::iterator it);
        std::shared_ptr<Reference> get_expression(std::shared_ptr<Parser::Expression> expression, Instructions & instructions, Instructions::iterator it);

        void write_structures(std::string & interface, std::string & implementation);
        void write_functions(std::string & interface, std::string & implementation);
        void write_main(std::string & implementation);

    public:

        Translator(std::shared_ptr<Parser::Expression> expression, Analyzer::MetaData const& meta_data):
            expression{ expression }, meta_data{ meta_data } {}

        void translate(std::filesystem::path const& out);

    };

}


#endif
