#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>

#include "Translator.hpp"

#include "../Functions.hpp"


namespace CTranslator {

    struct CorrespondanceTable: public std::map<std::shared_ptr<Analyzer::Type>, std::shared_ptr<Structures::Type>> {

        CorrespondanceTable() {
            this->operator[](Analyzer::Bool) = Structures::Bool;
            this->operator[](Analyzer::Char) = Structures::Char;
            this->operator[](Analyzer::Int) = Structures::Int;
            this->operator[](Analyzer::Float) = Structures::Float;
        }

    };

    std::pair<std::set<std::shared_ptr<Structures::Component>>, std::set<std::shared_ptr<Structures::Class>>> create_structures(std::vector<std::shared_ptr<Analyzer::Structure>> structures) {

        std::map<std::pair<std::string, std::weak_ptr<Analyzer::Type>>, std::set<std::shared_ptr<Analyzer::Structure>>> properties;
        for (auto const& s : structures) {
            for (auto const& p : s->properties) {
                for (auto const& t : p.second) {
                    properties[{p.first, t}].insert(s);
                }
            }
        }

        std::map<std::set<std::shared_ptr<Analyzer::Structure>>, std::map<std::string, std::weak_ptr<Analyzer::Type>>> groups;
        for (auto const& p : properties) {
            groups[p.second][p.first.first] = p.first.second;
        }

        std::map<std::map<std::string, std::weak_ptr<Analyzer::Type>>, std::set<std::shared_ptr<Analyzer::Structure>>> compos;
        for (auto const& g : groups) {
            compos[g.second] = g.first;
        }

        CorrespondanceTable map;
        std::set<std::shared_ptr<Structures::Component>> components;
        std::map<std::shared_ptr<Analyzer::Structure>, std::set<std::weak_ptr<Structures::Component>>> used_components;
        for (auto const& c : compos) {
            auto component = std::make_shared<Structures::Component>();
            for (auto const& p : c.first) {
                component->properties[p.first];
            }
            components.insert(component);

            for (auto const& s : c.second) {
                used_components[s].insert(component);
            }
        }
        std::set<std::shared_ptr<Structures::Class>> classes;
        for (auto const& uc : used_components) {
            auto cl = std::make_shared<Structures::Class>();
            cl->components = uc.second;
            classes.insert(cl);
            map[uc.first] = cl;
        }
    }

    void get_instructions(std::shared_ptr<Parser::Expression> expression, Analyzer::MetaData & meta, Instructions & instructions, References & references) {
        if (auto exp = std::dynamic_pointer_cast<Structures::Instruction>(get_expression(expression, meta, instructions, references)))
            instructions.push_back(exp);
    }

    std::shared_ptr<Structures::Expression> get_expression(std::shared_ptr<Analyzer::AnalyzedExpression> expression, Instructions & instructions) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            auto & link = meta.links[function_call];
            if (link.size() == 1) {
                if (auto f = std::get_if<std::shared_ptr<FunctionDefinition>>(&*link.begin())) {

                    auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                        .function = get_expression(function_call->function, meta, instructions, references)
                    });

                    if (std::dynamic_pointer_cast<Parser::Tuple>((*f)->parameters)) {
                        if (auto args = std::dynamic_pointer_cast<Parser::Tuple>(function_call->arguments)) {
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

                auto & type = meta.types[function_call->arguments];
                if (type.size() == 1) {
                    r->parameters.push_back(
                        std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                            .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                .name = "__UnknownData_from_data"
                            }),
                            .parameters = {
                                std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                    .name = "&__VirtualTable_" + (*type.begin()).get().name
                                }),
                                get_expression(function_call->arguments, meta, instructions, references)
                            }
                        })
                    );
                } else {
                    r->parameters.push_back(get_expression(function_call->arguments, meta, instructions, references));
                }

                return r;
            }
        } else if (auto function_run = std::dynamic_pointer_cast<Analyzer::FunctionRun>(expression)) {
            auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                .function = function_run->function
            });

            if (auto args = std::dynamic_pointer_cast<Analyzer::Tuple>(function_run->arguments)) {
                for (auto const& o : args->objects)
                    r->parameters.push_back(get_expression(o, instructions));
            } else {
                r->parameters.push_back(get_expression(function_run->arguments, instructions));
            }

            return r;
        } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
            Structures::FunctionDefinition::Parameters parameters;
            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(function_definition->parameters)) {
                unsigned int i = 0;
                for (auto p : tuple->objects) {
                    auto & types = meta.types[p];
                    auto & type = (types.size() == 1) ? *references.types[*types.begin()] : Structures::Unknown;

                    std::string name;
                    if (auto s = std::dynamic_pointer_cast<Parser::Symbol>(p))
                        name = s->name;
                    else
                        name = "arg" + std::to_string(i);

                    parameters.push_back({name, type});

                    i++;
                }
            } else {
                auto types = meta.types[function_definition->parameters];
                auto & type = (types.size() == 1) ? *references.types[*types.begin()] : Structures::Unknown;

                std::string name;
                if (auto s = std::dynamic_pointer_cast<Parser::Symbol>(function_definition->parameters))
                    name = s->name;
                else
                    name = "arg0";

                parameters.push_back({name, type});
            }

            Structures::Block body;
            get_instructions(function_definition->body, meta, body, references);

            Structures::Declarations local_variables;
            for (auto & var : meta.variables[function_definition])
                local_variables[var.first] = (var.second.size() == 1) ? *references.types[*var.second.begin()] : Structures::Unknown;

            auto & types = meta.types[function_definition];
            Structures::FunctionDefinition function {
                .type = types.size() == 1 ? *references.types[*types.begin()] : Structures::Unknown,
                .name = meta.names[function_definition],
                .parameters = parameters,
                .local_variables = local_variables,
                .body = body
            };

            references.functions[function_definition] = function;
        } else if (auto property = std::dynamic_pointer_cast<Analyzer::Property>(expression)) {
            auto o = get_expression(property->object, instructions);
            return std::make_shared<Structures::Property>(Structures::Property {
                .object = o,
                .name = property->name,
                .pointer = true
            });
        } else if (auto symbol = std::dynamic_pointer_cast<Analyzer::Symbol>(expression)) {
            return std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                .name = symbol->name
            });
        } else if (auto tuple = std::dynamic_pointer_cast<Analyzer::Tuple>(expression)) {
            auto list = std::make_shared<Structures::List>(Structures::List {});
            for (auto const& o : tuple->objects)
                list->objects.push_back(get_expression(o, instructions));
        }
    }

}
