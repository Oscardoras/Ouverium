#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "Structures.hpp"

#include "../Analyzer.hpp"


namespace CTranslator {

    struct References {
        std::map<std::reference_wrapper<Analyzer::MetaData::Type>, Structures::Type> types;
        std::map<std::shared_ptr<FunctionDefinition>, Structures::FunctionDefinition> functions;
    };
    using Instructions = std::vector<std::shared_ptr<Structures::Instruction>>;

    Structures::Structure create_struct(Analyzer::MetaData::Structure const& structure);

    std::shared_ptr<Structures::Expression> eval_system_function(Analyzer::SystemFunction function, std::shared_ptr<Expression> arguments, Analyzer::MetaData & meta, Instructions & instructions, References & references);

    void get_instructions(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions, References & references);
    std::shared_ptr<Structures::Expression> get_expression(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions, References & references);

}


#endif
