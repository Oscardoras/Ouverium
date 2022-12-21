#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "Structures.hpp"

#include "../Analyzer.hpp"


namespace CTranslator {

    using Instructions = std::vector<std::shared_ptr<Structures::Instruction>>;

    std::shared_ptr<Structures::Expression> eval_system_function(Interpreter::Reference (*function)(Interpreter::FunctionContext&), std::shared_ptr<Expression> arguments, Analyzer::MetaData & meta, Instructions & instructions);

    void get_instructions(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions);
    std::shared_ptr<Structures::Expression> get_expression(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions);

}


#endif
