#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

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

        Analyzer::MetaData meta_data;
        TypeTable type_table;
        FunctionTable function_table;

        std::shared_ptr<Expression> get_unknown_data(std::shared_ptr<Expression> expression);

        C c;

    public:

        std::set<std::shared_ptr<Class>> create_structures(std::set<std::shared_ptr<Analyzer::Structure>> const& structures);
        std::shared_ptr<FunctionDefinition> get_function(std::shared_ptr<Parser::FunctionDefinition> function);

        std::shared_ptr<Reference> eval_system_function(Analyzer::SystemFunction const& function, std::shared_ptr<Parser::Expression> arguments, Instructions & instructions, Instructions::iterator it);
        std::shared_ptr<Reference> get_expression(std::shared_ptr<Parser::Expression> expression, Instructions & instructions, Instructions::iterator it);

        void write_structures(std::string & interface, std::string & implementation);
        void write_functions(std::string & interface, std::string & implementation);
        void write_main(std::string & implementation);

    };

}


#endif
