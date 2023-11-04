#include <algorithm>
#include <hash_string.h>
#include <map>
#include <set>
#include <stdexcept>

#include "Translator.hpp"
#include "../../Utils.hpp"


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

    std::shared_ptr<FunctionDefinition> Translator::get_function(std::shared_ptr<Parser::FunctionDefinition> function_definition) {
        auto function = std::make_shared<FunctionDefinition>();

        function->return_value = get_expression(function_definition->body, function->body, function->body.begin());

        return function;
    }

    auto UnknownData_from_data(std::shared_ptr<Type> type, std::shared_ptr<Expression> value) {
        return std::make_shared<FunctionCall>(FunctionCall {
            std::make_shared<Symbol>("__UnknownData_from_data"),
            {
                std::make_shared<Symbol>("&__VirtualTable_" + type->name),
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
            auto args =  get_expression(function_call->arguments, instructions, it);

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
                        std::make_shared<FunctionExpression>(
                            args
                        )
                    }
                })
            ));

            return r;
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto r = std::make_shared<Reference>(true);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                get_function(function_definition)
            ));

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
                if (auto c = std::get_if<char>(&v)) {
                    value = std::make_shared<Value>(*c);
                    type = Char;
                } else if (auto b = std::get_if<bool>(&v)) {
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
