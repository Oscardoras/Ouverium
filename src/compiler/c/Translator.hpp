#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "Code.hpp"

#include "../Analyzer.hpp"


namespace Translator {

    namespace CStandard {

        struct CorrespondanceTable: public std::map<std::shared_ptr<Analyzer::Type>, std::shared_ptr<Type>> {

            CorrespondanceTable() {
                this->operator[](Analyzer::Bool) = Bool;
                this->operator[](Analyzer::Char) = Char;
                this->operator[](Analyzer::Int) = Int;
                this->operator[](Analyzer::Float) = Float;
            }

        };

        using Instructions = std::vector<std::shared_ptr<Instruction>>;

        class Translator {

            CorrespondanceTable table;

        public:

            std::pair<std::set<std::shared_ptr<Component>>, std::set<std::shared_ptr<Class>>> create_structures(std::vector<std::shared_ptr<Analyzer::Structure>> structures);

            std::shared_ptr<Expression> eval_system_function(Analyzer::SystemFunction function, std::shared_ptr<Parser::Expression> arguments, Instructions & instructions);

            void get_instructions(std::shared_ptr<Analyzer::Expression> expression, Instructions & instructions);
            std::shared_ptr<Expression> get_expression(std::shared_ptr<Analyzer::Expression> expression, Instructions & instructions);

        };

    }

}


#endif
