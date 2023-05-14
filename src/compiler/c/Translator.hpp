#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "Structures.hpp"

#include "../Analyzer.hpp"


namespace CTranslator {

    struct MetaData {
        std::vector<Structures::Structure> structures;
        std::vector<Structures::FunctionDefinition> functions;
    };
    using Instructions = std::vector<std::shared_ptr<Structures::Instruction>>;

    Structures::Structure create_struct(Analyzer::Structure const& structure);

    std::shared_ptr<Structures::Expression> eval_system_function(Analyzer::SystemFunction function, std::shared_ptr<Parser::Expression> arguments, Analyzer::MetaData & meta, Instructions & instructions, References & references);

    void get_instructions(std::shared_ptr<Parser::Expression> expression, Analyzer::MetaData & meta, Instructions & instructions);
    std::shared_ptr<Structures::Expression> get_expression(std::shared_ptr<Analyzer::AnalyzedExpression> expression, Instructions & instructions);

}


#endif
