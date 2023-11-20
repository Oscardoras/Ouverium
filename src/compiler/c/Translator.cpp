#include <algorithm>
#include <fstream>
#include <iostream>
#include <hash_string.h>
#include <map>
#include <set>
#include <stdexcept>

#include "Translator.hpp"
#include "../../Utils.hpp"


namespace Translator::CStandard {

    void Translator::translate(std::filesystem::path const& out) {
        create_structures();

        {
            add_system_function("_x3B", "");
        }

        get_expression(expression, code.main_instructions, code.main_instructions.begin());

        std::string structures_header;
        std::string structures_code;
        std::string functions_header;
        std::string main_code;
        write_structures(structures_header, structures_code);
        write_functions(functions_header, main_code);
        write_main(main_code);

        std::filesystem::create_directory(out);
        (std::ofstream((out / "structures.h").c_str()) << structures_header).close();
        (std::ofstream((out / "structures.c").c_str()) << structures_code).close();
        (std::ofstream((out / "functions.h").c_str()) << functions_header).close();
        (std::ofstream((out / "main.c").c_str()) << main_code).close();
    }

    void Translator::create_structures() {
        for (auto s : meta_data.structures) {
            auto cl = std::make_shared<Structure>(s->name);
            code.structures.insert(cl);
            type_table[s] = cl;
        }

        for (auto s : meta_data.structures) {
            auto cl = std::static_pointer_cast<Structure>(type_table[s]);

            for (auto const& p : s->properties) {
                if (p.second.size() == 1)
                    cl->properties[p.first] = type_table[p.second.begin()->lock()];
                else
                    cl->properties[p.first] = Unknown;
            }
            cl->function = s->function;
            if (s->array.size() == 1)
                cl->array = type_table[s->array.begin()->lock()];
            else
                cl->array = Unknown;
        }
    }

    void parse_parameters(std::shared_ptr<Parser::Expression> parameters, std::shared_ptr<FunctionDefinition> function) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            function->parameters.push_back(symbol->name);
            function->format += 'r';
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            for (auto e : tuple->objects)
                parse_parameters(e, function);
        } else {
            // TODO
        }
    }

    void Translator::add_system_function(std::string symbol, std::string function, Instructions & instructions, Instructions::iterator it) {
        auto r = std::make_shared<Reference>(true);

        instructions.insert(it, std::make_shared<Affectation>(
            r,
            std::make_shared<FunctionCall>(FunctionCall {
                std::make_shared<Symbol>("__GC_alloc_object"),
                {
                    std::make_shared<Referencing>(std::make_shared<Symbol>("__VirtualTable_Function"))
                }
            })
        ));

        instructions.insert(it,
            std::make_shared<FunctionCall>(FunctionCall {
                std::make_shared<Symbol>("__Function_push"),
                {
                    std::make_shared<FunctionCall>(FunctionCall {
                        std::make_shared<Symbol>("__UnknownData_get_function"),
                        {
                            std::make_shared<FunctionCall>(FunctionCall {
                                std::make_shared<Symbol>("__Reference_get"),
                                {
                                    r
                                }
                            })
                        }
                    }),
                    std::make_shared<Symbol>(function = "_body"),
                    std::make_shared<Symbol>(function = "_filter"),
                    std::make_shared<List>(),
                    std::make_shared<Value>(0)
                }
            })
        );
    }

    std::shared_ptr<FunctionDefinition> Translator::create_function(std::shared_ptr<Parser::FunctionDefinition> function_definition) {
        auto function = std::make_shared<FunctionDefinition>();

        for (auto symbol : function_definition->captures)
            function->captures.push_back(symbol);

        parse_parameters(function_definition->parameters, function);

        if (function_definition->filter != nullptr)
            function->filter.return_value = get_expression(function_definition->filter, function->filter.body, function->filter.body.begin());

        function->body.return_value = get_expression(function_definition->body, function->body.body, function->body.body.begin());

        code.functions.insert(function);
        return function;
    }

    auto UnknownData_from_data(std::shared_ptr<Type> type, std::shared_ptr<Expression> value) {
        return std::make_shared<FunctionCall>(FunctionCall {
            std::make_shared<Symbol>("__UnknownData_from_data"),
            {
                std::make_shared<Referencing>(std::make_shared<Symbol>("__VirtualTable_" + type->name)),
                value
            }
        });
    }

    std::shared_ptr<Reference> Translator::get_expression(std::shared_ptr<Parser::Expression> expression, Instructions & instructions, Instructions::iterator it) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            /*
            if (meta_data.calls[function_call].size() == 1) {
                auto const& function = *meta_data.calls[function_call].begin();

                if (auto system_function = std::get_if<Analyzer::SystemFunction>(&function)) {
                    return eval_system_function(*system_function, function_call->arguments, instructions, it);
                }
            }
            */

            auto r = std::make_shared<Reference>(true);

            auto f = get_expression(function_call->function, instructions, it);
            auto args = std::make_shared<FunctionExpression>(get_expression(function_call->arguments, instructions, it));

            instructions.insert(it, args);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__Function_eval"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall {
                            std::make_shared<Symbol>("__UnknownData_get_function"),
                            {
                                std::make_shared<FunctionCall>(FunctionCall {
                                    std::make_shared<Symbol>("__Reference_get"),
                                    {
                                        std::make_shared<FunctionCall>(FunctionCall {
                                            std::make_shared<Symbol>("__Reference_share"),
                                            {
                                                f
                                            }
                                        })
                                    }
                                })
                            }
                        }),
                        args
                    }
                })
            ));

            return r;
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto r = std::make_shared<Reference>(true);

            auto function = create_function(function_definition);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__GC_alloc_object"),
                    {
                        std::make_shared<Referencing>(std::make_shared<Symbol>("__VirtualTable_Function"))
                    }
                })
            ));

            std::vector<std::shared_ptr<Expression>> captures;
            for (auto symbol : function_definition->symbols) {
                auto symbols = function_definition->parent.lock()->symbols;
                if (symbols.find(symbol) != symbols.end())
                    captures.push_back(std::make_shared<Symbol>(symbol));
            }

            instructions.insert(it,
                std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__Function_push"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall {
                            std::make_shared<Symbol>("__UnknownData_get_function"),
                            {
                                std::make_shared<FunctionCall>(FunctionCall {
                                    std::make_shared<Symbol>("__Reference_get"),
                                    {
                                        r
                                    }
                                })
                            }
                        }),
                        function
                    }
                })
            );

            return r;
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            auto r = std::make_shared<Reference>(false);

            auto parent = get_expression(property->object, instructions, it);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__UnknownData_get_property"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall {
                            std::make_shared<Symbol>("__Reference_get"),
                            {
                                std::make_shared<FunctionCall>(FunctionCall {
                                    std::make_shared<Symbol>("__Reference_share"),
                                    {
                                        parent
                                    }
                                })
                            }
                        }),
                        std::make_shared<Value>(static_cast<long>(hash(property->name.c_str())))
                    }
                })
            ));

            if (parent->owned) {
                instructions.insert(it, std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__Reference_free"),
                    {
                        parent
                    }
                }));
            }

            return r;
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto v = get_symbol(symbol->name);

            if (std::holds_alternative<nullptr_t>(v)) {
                auto r = std::make_shared<Reference>(false);

                instructions.insert(it, std::make_shared<Affectation>(
                    r,
                    std::make_shared<Symbol>(symbol->name)
                ));

                return r;
            } else {
                auto r = std::make_shared<Reference>(true);

                std::shared_ptr<Value> value = nullptr;
                std::shared_ptr<Type> type = nullptr;
                if (auto b = std::get_if<bool>(&v)) {
                    value = std::make_shared<Value>(*b);
                    type = Bool;
                } else if (auto l = std::get_if<long>(&v)) {
                    value = std::make_shared<Value>(*l);
                    type = Int;
                } else if (auto d = std::get_if<double>(&v)) {
                    value = std::make_shared<Value>(*d);
                    type = Float;
                }

                instructions.insert(it, std::make_shared<Affectation>(
                    r,
                    std::make_shared<FunctionCall>(FunctionCall {
                        std::make_shared<Symbol>("__Reference_new_data"),
                        {
                            UnknownData_from_data(
                                type,
                                value
                            )
                        }
                    })
                ));

                return r;
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            auto r = std::make_shared<Reference>(true);

            std::vector<std::shared_ptr<Reference>> tmp;
            auto list = std::make_shared<List>();
            for (auto const& o : tuple->objects) {
                auto ref = get_expression(o, instructions, it);
                if (ref->owned)
                    tmp.push_back(ref);
                list->objects.push_back(ref);
            }

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__Reference_new_tuple"),
                    {
                        list,
                        std::make_shared<Value>(static_cast<long>(list->objects.size()))
                    }
                })
            ));

            for (auto ref : tmp) {
                instructions.insert(it, std::make_shared<FunctionCall>(FunctionCall {
                    std::make_shared<Symbol>("__Reference_free"),
                    {
                        ref
                    }
                }));
            }

            return r;
        } else return nullptr;
    }

}
