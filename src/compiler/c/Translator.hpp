#ifndef __COMPILER_C_TRANSLATOR_HPP__
#define __COMPILER_C_TRANSLATOR_HPP__

#include <list>
#include <variant>

#include "Structures.hpp"

#include "../Analyzer.hpp"


namespace CTranslator {

    std::vector<std::shared_ptr<CStructures::Instruction>> eval_system_function(Interpreter::Reference (*function)(Interpreter::FunctionContext&), std::shared_ptr<Expression> arguments, Analyzer::MetaData & meta);

    std::vector<std::shared_ptr<CStructures::Instruction>> get_instructions(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta);
    std::shared_ptr<CStructures::Expression> get_expression(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta);

}


#endif
