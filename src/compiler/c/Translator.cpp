#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>

#include "Translator.hpp"

#include "../Functions.hpp"


namespace Translator::CStandard {

    std::set<std::shared_ptr<Class>> Translator::create_structures(std::set<std::shared_ptr<Analyzer::Structure>> const& structures) {
        std::set<std::shared_ptr<Class>> classes;

        for (auto s : structures) {
            auto cl = std::make_shared<Class>();
            classes.insert(cl);
            type_table[s] = cl;
        }

        for (auto s : structures) {
            auto cl = std::static_pointer_cast<Class>(type_table[s]);
            for (auto const& p : s->properties) {
                if (p.second.size() == 1) {
                    cl->properties[p.first] = type_table[p.second.begin()->lock()];
                } else {
                    cl->properties[p.first] = Unknown;
                }
            }
        }

        return classes;
    }

    std::set<std::shared_ptr<FunctionDefinition>> Translator::create_functions(std::set<std::shared_ptr<Analyzer::FunctionDefinition>> const& functions) {
        std::set<std::shared_ptr<FunctionDefinition>> funcs;

        for (auto const& f : functions) {
            auto func = std::make_shared<FunctionDefinition>();
            funcs.insert(func);
            function_table[f] = func;
        }

        return funcs;
    }

    std::shared_ptr<FunctionDefinition> Translator::get_function(std::shared_ptr<Analyzer::FunctionDefinition> function_definition) {
        FunctionDefinition::Parameters parameters;
        if (auto tuple = std::dynamic_pointer_cast<Analyzer::Tuple>(function_definition->parameters)) {
            size_t i = 0;
            for (auto p : tuple->objects) {
                std::string name;
                if (auto s = std::dynamic_pointer_cast<Parser::Symbol>(p))
                    name = s->name;
                else
                    name = "arg" + std::to_string(i);

                parameters.push_back({name, type_table.get(p->types)});
                ++i;
            }
        } else {
            std::string name;
            if (auto s = std::dynamic_pointer_cast<Parser::Symbol>(function_definition->parameters))
                name = s->name;
            else
                name = "arg0";

            parameters.push_back({name, type_table.get(function_definition->parameters->types)});
        }

        Block body;
        get_instructions(function_definition->body, body);

        Declarations local_variables;
        for (auto & var : function_definition->local_variables)
            local_variables[var.first] = type_table.get(var.second);

        return function_table[function_definition] = std::make_shared<FunctionDefinition>(FunctionDefinition {
            .type = type_table.get(function_definition->body->types),
            .name = "TODO : set name",
            .parameters = parameters,
            .local_variables = local_variables,
            .body = body
        });
    }

    void Translator::get_instructions(std::shared_ptr<Analyzer::Expression> expression, Instructions & instructions) {
        if (auto exp = std::dynamic_pointer_cast<Instruction>(get_expression(expression, instructions)))
            instructions.push_back(exp);
    }

    auto UnknownData_from_data(std::shared_ptr<Type> type, std::shared_ptr<Expression> value) {
        return std::make_shared<FunctionCall>(FunctionCall {
            .function = std::make_shared<VariableCall>(VariableCall {
                .name = "__UnknownData_from_data"
            }),
            .parameters = {
                std::make_shared<VariableCall>(VariableCall {
                    .name = "&__VirtualTable_" + type->name
                }),
                value
            }
        });
    }

    std::shared_ptr<Expression> Translator::get_unknown_data(std::shared_ptr<Expression> expression) {
        if (auto type = expression->type.lock()) {
            return UnknownData_from_data(type, expression);
        } else {
            return expression;
        }
    }

    std::shared_ptr<Expression> Translator::get_expression(std::shared_ptr<Analyzer::Expression> expression, Instructions & instructions) {
        if (auto function_call = std::dynamic_pointer_cast<Analyzer::FunctionCall>(expression)) {
            auto r = std::make_shared<FunctionCall>(FunctionCall {
                .function = std::make_shared<VariableCall>(VariableCall {
                    .name = "__Function_eval"
                })
            });
            r->type = Unknown;

            auto function = get_expression(function_call->function, instructions);
            if (auto type = function->type.lock()) {
                r->parameters.push_back(std::make_shared<Property>(Property {
                    {
                        type_table.get(expression->types)
                    },
                    function,
                    "function_stack",
                    false
                }));
            } else {
                r->parameters.push_back(std::make_shared<FunctionCall>(FunctionCall {
                    .function = std::make_shared<VariableCall>(VariableCall {
                        .name = "__UnknownData_get_component"
                    }),
                    .parameters = {
                        std::make_shared<VariableCall>(VariableCall {
                            .name = "__Function_Stack"
                        }),
                        function
                    }
                }));
            }

            r->parameters.push_back(get_unknown_data(get_expression(function_call->arguments, instructions)));

            return r;
        } else if (auto function_run = std::dynamic_pointer_cast<Analyzer::FunctionRun>(expression)) {
            if (auto system_function = std::get_if<Analyzer::SystemFunction>(&function_run->function)) {
                return eval_system_function(*system_function, function_run->arguments, instructions);
            } else if (auto function_definition = std::get_if<std::weak_ptr<Analyzer::FunctionDefinition>>(&function_run->function)) {
                auto function = function_table[function_definition->lock()];

                auto r = std::make_shared<FunctionCall>(FunctionCall {
                    .function = std::make_shared<VariableCall>(VariableCall {
                        .name = function->name
                    })
                });

                if (function->parameters.size() > 1) {
                    if (auto a_tuple = std::dynamic_pointer_cast<Analyzer::Tuple>(function_run->arguments)) {
                        if (a_tuple->objects.size() == function->parameters.size()) {

                        }
                    } else {

                    }
                } else {

                }
                if (auto args = std::dynamic_pointer_cast<Analyzer::Tuple>(function_run->arguments)) {
                    for (auto const& o : args->objects)
                        r->parameters.push_back(get_expression(o, instructions));
                } else {
                    r->parameters.push_back(get_expression(function_run->arguments, instructions));
                }

                return r;
            }
        } else if (auto property = std::dynamic_pointer_cast<Analyzer::Property>(expression)) {
            auto o = get_expression(property->object, instructions);

            if (auto type = o->type.lock()) {
                std::shared_ptr<Component> component;
                for (auto const& c : std::dynamic_pointer_cast<Class>(type)->components) {
                    auto comp = c.lock();
                    if (comp->properties.find(property->name) != comp->properties.end()) {
                        component = comp;
                        break;
                    }
                }

                return std::make_shared<Property>(Property {
                    {
                        type_table.get(expression->types)
                    },
                    std::make_shared<Property>(Property {
                        .object = o,
                        .name = component->name,
                        .pointer = true
                    }),
                    property->name,
                    false
                });
            } else {
                std::map<std::shared_ptr<Component>, std::set<std::shared_ptr<Class>>> components;
                for (auto const& t : property->object->types)
                    if (auto type = std::dynamic_pointer_cast<Class>(type_table[t.lock()]))
                        for (auto const& c : type->components) {
                            auto component = c.lock();
                            auto it = component->properties.find(property->name);
                            if (it != component->properties.end())
                                components[component].insert(type);
                        }
                if (components.size() == 1) {
                    auto component = components.begin()->first;

                    return std::make_shared<Property>(Property {
                        {
                            type_table.get(expression->types)
                        },
                        UnknownData_from_data(type_table[function_call->arguments->types.begin()->lock()], get_expression(function_call->arguments, instructions)),
                        property->name,
                        true
                    });
                } else {
                    // TODO
                }
            }
        } else if (auto symbol = std::dynamic_pointer_cast<Analyzer::Symbol>(expression)) {
            return std::make_shared<VariableCall>(VariableCall {
                {
                    type_table.get(expression->types)
                },
                symbol->name
            });
        } else if (auto tuple = std::dynamic_pointer_cast<Analyzer::Tuple>(expression)) {
            auto list = std::make_shared<List>(List {
                {
                    type_table.get(expression->types)
                }
            });
            for (auto const& o : tuple->objects)
                list->objects.push_back(get_expression(o, instructions));
        } else if (auto value = std::dynamic_pointer_cast<Analyzer::Value>(expression)) {
            return std::make_shared<Value>(Value {
                {
                    type_table.get(expression->types)
                },
                value->value
            });
        } else return nullptr;
    }

}
