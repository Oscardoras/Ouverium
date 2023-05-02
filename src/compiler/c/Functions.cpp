#include <algorithm>
#include <map>
#include <stdexcept>

#include "Translator.hpp"

#include "../Functions.hpp"


namespace CTranslator {

    std::shared_ptr<Structures::Expression> eval_system_function(Analyzer::SystemFunction function, std::shared_ptr<Parser::Expression> arguments, Analyzer::MetaData & meta, Instructions & instructions, References & references) {
        if (function.pointer == Analyzer::Functions::separator) {
            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
                for (auto const& o : tuple->objects)
                    get_instructions(o, meta, instructions, references);
            } else {
                get_instructions(arguments, meta, instructions, references);
            }
        }
        else if (function.pointer == Analyzer::Functions::if_statement) {
            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
                if (tuple->objects.size() >= 2) {
                    int i = 0;
                    auto structure = std::make_shared<Structures::If>();
                    structure->condition = get_expression(tuple->objects[i++], meta, instructions, references);
                    get_instructions(tuple->objects[i++], meta, structure->body, references);

                    while (i+2 < tuple->objects.size()) {
                        i++;
                        auto tmp = std::make_shared<Structures::If>();
                        tmp->condition = get_expression(tuple->objects[i++], meta, structure->alternative, references);
                        get_instructions(tuple->objects[i++], meta, tmp->body, references);
                        structure->alternative.push_back(tmp);
                        structure = tmp;
                    };

                    if (i+1 < tuple->objects.size()) {
                        i++;
                        get_instructions(tuple->objects[i++], meta, structure->alternative, references);
                    }

                    instructions.push_back(structure);
                }
            }
        }
        else if (function.pointer == Analyzer::Functions::while_statement) {
            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
                if (tuple->objects.size() == 2) {
                    auto condition = tuple->objects[0];
                    auto body = tuple->objects[1];

                    auto structure = std::make_shared<Structures::While>(Structures::While {
                        .condition = get_expression(condition, meta, instructions, references),
                    });
                    get_instructions(body, meta, structure->body, references);
                    instructions.push_back(structure);
                }
            }
        }
        else if (function.pointer == Analyzer::Functions::copy) {
            return std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {.name = ""}),
                .parameters = std::vector<std::shared_ptr<Structures::Expression>> { get_expression(arguments, meta, instructions, references) }
            });
        }
        else if (function.pointer == Analyzer::Functions::copy_pointer) {
            return std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {.name = ""}),
                .parameters = std::vector<std::shared_ptr<Structures::Expression>> { get_expression(arguments, meta, instructions, references) }
            });
        }
        else if (function.pointer == Analyzer::Functions::assign) {
            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
                if (tuple->objects.size() == 2) {
                    auto var = get_expression(tuple->objects[0], meta, instructions, references);
                    auto object = get_expression(tuple->objects[1], meta, instructions, references);

                    if (auto lvalue = std::dynamic_pointer_cast<Structures::LValue>(var)) {
                        return std::make_shared<Structures::Affectation>(Structures::Affectation {
                            .lvalue = lvalue,
                            .value = object
                        });
                    } else if (auto var_tuple = std::dynamic_pointer_cast<Parser::Tuple>(tuple->objects[0])) {
                        if (auto object_tuple = std::dynamic_pointer_cast<Parser::Tuple>(tuple->objects[1])) {
                            if (var_tuple->objects.size() == object_tuple->objects.size()) {
                                for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
                                    instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation {
                                        .lvalue = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                            .name = "tmp_" +
                                        }),
                                        .value = get_expression(object_tuple->objects[i], meta, instructions, references)
                                    }));
                                }
                                for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
                                    instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation {
                                        .lvalue = get_expression(var_tuple->objects[i], meta, instructions, references),
                                        .value = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                            .name = "tmp_" +
                                        })
                                    }));
                                }
                            }
                        }
                    }

                    return var;
                }
            }
        }
    }

}
