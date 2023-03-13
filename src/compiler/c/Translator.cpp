#include <algorithm>
#include <map>
#include <stdexcept>

#include "Translator.hpp"

#include "../Functions.hpp"


namespace CTranslator {

    Structures::Structure create_struct(Analyzer::MetaData::Structure const& structure) {
        Structures::Structure s;

        for (auto const& pair : structure) {
            if (pair.second.size() == 1) {
                auto ref = *pair.second.begin();
                if (ref.get() == Analyzer::MetaData::Pointer)
                    s.properties[pair.first] = Structures::Pointer;
                else if (ref.get() == Analyzer::MetaData::Bool)
                    s.properties[pair.first] = Structures::Bool;
                else if (ref.get() == Analyzer::MetaData::Char)
                    s.properties[pair.first] = Structures::Char;
                else if (ref.get() == Analyzer::MetaData::Int)
                    s.properties[pair.first] = Structures::Int;
                else if (ref.get() == Analyzer::MetaData::Float)
                    s.properties[pair.first] = Structures::Float;
            } else {
                s.properties[pair.first] = Structures::Unknown;
            }
        }

        return s;
    }

    std::shared_ptr<Structures::Expression> eval_system_function(Analyzer::SystemFunction function, std::shared_ptr<Expression> arguments, Analyzer::MetaData & meta, Instructions & instructions, References & references) {
        switch ((unsigned long) function.pointer) {
        case (unsigned long) Analyzer::Functions::separator:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                for (auto const& o : tuple->objects)
                    get_instructions(o, meta, instructions);
            } else {
                get_instructions(arguments, meta, instructions);
            }
            break;

        case (unsigned long) Analyzer::Functions::if_statement:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (tuple->objects.size() >= 2) {
                    int i = 0;
                    auto structure = std::make_shared<Structures::If>();
                    structure->condition = get_expression(tuple->objects[i++], meta, instructions);
                    get_instructions(tuple->objects[i++], meta, structure->body);

                    while (i+2 < tuple->objects.size()) {
                        i++;
                        auto tmp = std::make_shared<Structures::If>();
                        tmp->condition = get_expression(tuple->objects[i++], meta, structure->alternative);
                        get_instructions(tuple->objects[i++], meta, tmp->body);
                        structure->alternative.push_back(tmp);
                        structure = tmp;
                    };

                    if (i+1 < tuple->objects.size()) {
                        i++;
                        get_instructions(tuple->objects[i++], meta, structure->alternative);
                    }

                    instructions.push_back(structure);
                }
            }
            break;

        case (unsigned long) Interpreter::Base::while_statement:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (tuple->objects.size() == 2) {
                    auto condition = tuple->objects[0];
                    auto body = tuple->objects[1];

                    auto structure = std::make_shared<Structures::While>(Structures::While {
                        .condition = get_expression(condition, meta, instructions),
                    });
                    get_instructions(body, meta, structure->body);
                    instructions.push_back(structure);
                }
            }
            break;

        case (unsigned long) Interpreter::Base::copy_pointer:

            break;

        case (unsigned long) Interpreter::Base::assign:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (tuple->objects.size() == 2) {
                    auto var = get_expression(tuple->objects[0], meta, instructions);
                    auto object = get_expression(tuple->objects[1], meta, instructions);

                    if (auto r_value = std::dynamic_pointer_cast<Structures::RValue>(var)) {
                        return std::make_shared<Structures::Affectation>(Structures::Affectation {
                            .r_value = r_value,
                            .value = object
                        });
                    } else if (auto var_tuple = std::dynamic_pointer_cast<Tuple>(tuple->objects[0])) {
                        if (auto object_tuple = std::dynamic_pointer_cast<Tuple>(tuple->objects[1])) {
                            if (var_tuple->objects.size() == object_tuple->objects.size()) {
                                for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
                                    instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation {
                                        .r_value = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                            .name = "tmp_" +
                                        }),
                                        .value = get_expression(object_tuple->objects[i], meta, instructions)
                                    }));
                                }
                                for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
                                    instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation {
                                        .r_value = get_expression(var_tuple->objects[i], meta, instructions),
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
            break;

        default:
            break;
        }
    }

    void get_instructions(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions, References & references) {
        if (auto exp = std::dynamic_pointer_cast<Structures::Instruction>(get_expression(expression, meta, instructions, references)))
            instructions.push_back(exp);
    }

    std::shared_ptr<Structures::Expression> get_expression(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions, References & references) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            auto & link = meta.links[function_call];
            if (link.size() == 1) {
                if (auto f = std::get_if<std::shared_ptr<FunctionDefinition>>(&*link.begin())) {

                    auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                        .function = get_expression(function_call->function, meta, instructions, references)
                    });

                    if (std::dynamic_pointer_cast<Tuple>((*f)->parameters)) {
                        if (auto args = std::dynamic_pointer_cast<Tuple>(function_call->arguments)) {
                            for (auto const& o : args->objects)
                                r->parameters.push_back(get_expression(o, meta, instructions, references));
                        }
                    } else {
                        r->parameters.push_back(get_expression(function_call->arguments, meta, instructions, references));
                    }

                    return r;
                } else if (auto f = std::get_if<Analyzer::SystemFunction>(&*link.begin())) {
                    return eval_system_function(*f, function_call->arguments, meta, instructions);
                }
            } else {
                auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                    .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                        .name = "__Function_eval"
                    })
                });

                // TODO: context
                r->parameters.push_back(get_expression(function_call->function, meta, instructions, references));
                r->parameters.push_back(get_expression(function_call->arguments, meta, instructions, references));

                return r;
            }
        } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
            auto & types = meta.types[function_definition];

            std::vector<Structures::Declaration> parameters;
            if () {
                for (auto p : function_definition->parameters) {
                    
                }
            }

            Instructions function_instructions;
            get_instructions(function_definition->body, meta, function_instructions, references);

            Structures::FunctionDefinition function {
                .type = types.size() == 1 ? references.types[*types.begin()] : Structures::Unknown,
                .name = "" /* TODO */,
                .parameters = ,
                .body = function_instructions
            };

            references.functions[function_definition] = function;
        } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
            auto o = get_expression(property->object, meta, instructions, references);
            return std::make_shared<Structures::Property>(Structures::Property {
                .object = o,
                .name = property->name,
                .pointer = true
            });
        } else if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
            return std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                .name = symbol->name
            });
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
            auto list = std::make_shared<Structures::List>(Structures::List {});
            for (auto const& o : tuple->objects)
                list->objects.push_back(get_expression(o, meta, instructions, references));
        }
    }

}
