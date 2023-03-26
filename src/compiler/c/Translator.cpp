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
                    return eval_system_function(*f, function_call->arguments, meta, instructions, references);
                }
            } else {
                auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                    .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                        .name = "__Function_eval"
                    })
                });

                r->parameters.push_back(get_expression(function_call->function, meta, instructions, references));
                // TODO: context
                r->parameters.push_back(get_expression(function_call->arguments, meta, instructions, references));

                return r;
            }
        } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
            auto & types = meta.types[function_definition];

            std::vector<Structures::Declaration> parameters;
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(function_definition->parameters)) {
                int i = 0;
                for (auto p : tuple->objects) {
                    auto types = meta.types[p];
                    auto type = (types.size() == 1) ? references.types[*types.begin()] : Structures::Unknown;

                    std::string name;
                    if (auto s = std::dynamic_pointer_cast<Symbol>(p))
                        name = s->name;
                    else
                        name = "arg" + std::to_string(i);

                    parameters.push_back(Structures::Declaration {
                        .type = type,
                        .name = name
                    });

                    i++;
                }
            }

            Instructions function_instructions;
            get_instructions(function_definition->body, meta, function_instructions, references);

            Structures::FunctionDefinition function {
                .type = types.size() == 1 ? references.types[*types.begin()] : Structures::Unknown,
                .name = "" /* TODO */,
                .parameters = parameters,
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
