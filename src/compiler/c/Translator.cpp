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
        {
            create_structures();

            for (auto const& symbol : expression->symbols)
                if (std::holds_alternative<nullptr_t>(get_symbol(symbol)))
                    code.main.global_variables[symbol] = Unknown;

            code.main.return_value = get_expression(expression, code.main.body, code.main.body.end());
        }

        {
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
            function->parameters.push_back({ symbol->name, Unknown });
            function->format += 'r';
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            function->format += '[';
            for (auto e : tuple->objects)
                parse_parameters(e, function);
            function->format += ']';
        } else {
            // TODO
        }
    }

    std::shared_ptr<FunctionDefinition> Translator::create_function(std::shared_ptr<Parser::FunctionDefinition> function_definition) {
        auto function = std::make_shared<FunctionDefinition>();

        // captures
        for (auto const& symbol : function_definition->captures)
            function->captures.push_back({ symbol, Unknown });

        // arguments
        parse_parameters(function_definition->parameters, function);

        // local variables
        {
            std::set<std::string> symbols;
            for (auto const& [symbol, _] : function->captures)
                symbols.insert(symbol.symbol);
            for (auto const& [symbol, _] : function->parameters)
                symbols.insert(symbol.symbol);

            for (auto symbol : function_definition->symbols)
                if (!symbols.contains(symbol))
                    function->local_variables[symbol] = Unknown;
        }

        if (function_definition->filter)
            function->filter.return_value = get_expression(function_definition->filter, function->filter.body, function->filter.body.begin());

        function->body.return_value = get_expression(function_definition->body, function->body.body, function->body.body.begin());

        code.functions.insert(function);
        return function;
    }

    std::shared_ptr<Reference> Translator::get_expression(std::shared_ptr<Parser::Expression> expression, Instructions& instructions, Instructions::iterator it) {
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
            std::shared_ptr<FunctionExpression> args;

            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(function_call->function); symbol && std::set<std::string>{"if", "while"}.contains(symbol->name)) {

                std::function<std::shared_ptr<FunctionExpression>(std::shared_ptr<Parser::Expression>)> iterate = [this, &iterate](std::shared_ptr<Parser::Expression> expression) -> std::shared_ptr<FunctionExpression> {
                    if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
                        std::vector<std::shared_ptr<FunctionExpression>> vector;

                        for (auto const& e : tuple->objects)
                            vector.push_back(iterate(e));

                        return std::make_shared<FunctionExpression>(vector);
                    } else {
                        auto lambda = std::make_shared<Lambda>();
                        for (auto const& symbol : expression->symbols)
                            if (std::holds_alternative<nullptr_t>(get_symbol(symbol)))
                                lambda->captures.push_back({ symbol, Unknown });
                        lambda->body.return_value = get_expression(expression, lambda->body.body, lambda->body.body.begin());

                        if (!lambda->body.return_value->owned) {
                            auto result = std::make_shared<Reference>(true);
                            lambda->body.body.push_back(std::make_shared<Affectation>(
                                result,
                                std::make_shared<FunctionCall>(FunctionCall(
                                    std::make_shared<Symbol>("Ov_Reference_copy"),
                                    {
                                        lambda->body.return_value
                                    }
                                ))
                            ));
                            lambda->body.return_value = result;
                        }

                        code.lambdas.insert(lambda);
                        return std::make_shared<FunctionExpression>(lambda);
                    }
                };

                args = iterate(function_call->arguments);
            } else {
                args = std::make_shared<FunctionExpression>(get_expression(function_call->arguments, instructions, it));
            }

            instructions.insert(it, args);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Function_eval"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall(
                            std::make_shared<Symbol>("Ov_UnknownData_get_function"),
                            {
                                std::make_shared<FunctionCall>(FunctionCall(
                                    std::make_shared<Symbol>("Ov_Reference_get"),
                                    {
                                        std::make_shared<FunctionCall>(FunctionCall(
                                            std::make_shared<Symbol>("Ov_Reference_share"),
                                            {
                                                f
                                            }
                                        ))
                                    }
                                ))
                            }
                        )),
                        args
                    }
                ))
            ));

            if (f->owned) {
                instructions.insert(it, std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Reference_free"),
                    {
                        f
                    }
                )));
            }

            return r;
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto r = std::make_shared<Reference>(true);

            auto function = create_function(function_definition);

            std::string data_name = function->name.get() + "_data";
            instructions.insert(it, std::make_shared<Declaration>("Ov_Data", std::make_shared<Symbol>(data_name)));
            instructions.insert(it, std::make_shared<Affectation>(
                std::make_shared<Property>(
                    std::make_shared<Symbol>(data_name),
                    "ptr",
                    false
                ),
                std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_GC_alloc_object"),
                    {
                        std::make_shared<Referencing>(std::make_shared<Symbol>("Ov_VirtualTable_Function"))
                    }
                ))
            ));

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Reference_new_data"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall(
                            std::make_shared<Symbol>("Ov_UnknownData_from_data"),
                            {
                                std::make_shared<Referencing>(std::make_shared<Symbol>("Ov_VirtualTable_Function")),
                                std::make_shared<Symbol>(data_name)
                            }
                        ))
                    }
                ))
            ));

            auto captures = std::make_shared<List>();
            for (auto const& [capture, _] : function->captures)
                captures->objects.push_back(std::make_shared<Symbol>(capture));
            instructions.insert(it, captures);

            instructions.insert(it,
                std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Function_push"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall(
                            std::make_shared<Symbol>("Ov_UnknownData_get_function"),
                            {
                                std::make_shared<FunctionCall>(FunctionCall(
                                    std::make_shared<Symbol>("Ov_Reference_get"),
                                    {
                                        std::make_shared<FunctionCall>(FunctionCall(
                                            std::make_shared<Symbol>("Ov_Reference_share"),
                                            {
                                                r
                                            }
                                        ))
                                    }
                                ))
                            }
                        )),
                        std::make_shared<Value>(function->format),
                        std::make_shared<Symbol>(function->name.get() + "_body"),
                        function->filter.return_value ? std::make_shared<Symbol>(function->name.get() + "_filter") : std::make_shared<Symbol>("NULL"),
                        captures,
                        std::make_shared<Value>((long) captures->objects.size())
                    }
                ))
            );

            return r;
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            auto r = std::make_shared<Reference>(true);

            auto parent = get_expression(property->object, instructions, it);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Reference_new_property"),
                    {
                        std::make_shared<FunctionCall>(FunctionCall(
                            std::make_shared<Symbol>("Ov_Reference_get"),
                            {
                                std::make_shared<FunctionCall>(FunctionCall(
                                    std::make_shared<Symbol>("Ov_Reference_share"),
                                    {
                                        parent
                                    }
                                ))
                            }
                        )),
                        std::make_shared<Referencing>(std::make_shared<Symbol>("Ov_VirtualTable_UnknownData")),
                        std::make_shared<Value>(static_cast<long>(hash_string(property->name.c_str())))
                    }
                ))
            ));

            if (parent->owned) {
                instructions.insert(it, std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Reference_free"),
                    {
                        parent
                    }
                )));
            }

            return r;
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto v = get_symbol(symbol->name);

            if (std::holds_alternative<nullptr_t>(v)) {
                auto r = std::make_shared<Reference>(false);

                instructions.insert(it, std::make_shared<Affectation>(
                    r,
                    std::make_shared<FunctionCall>(FunctionCall(
                        std::make_shared<Symbol>("Ov_Reference_share"),
                        {
                            std::make_shared<Symbol>(symbol->name)
                        }
                    ))
                ));

                return r;
            } else if (std::holds_alternative<std::string>(v)) {
                auto r = std::make_shared<Reference>(true);

                instructions.insert(it, std::make_shared<Affectation>(
                    r,
                    std::make_shared<FunctionCall>(FunctionCall(
                        std::make_shared<Symbol>("Ov_Reference_new_string"),
                        {
                            std::make_shared<Value>(symbol->name),
                            std::make_shared<Referencing>(std::make_shared<Symbol>("Ov_VirtualTable_Object"))
                        }
                    ))
                ));

                return r;
            } else {
                auto r = std::make_shared<Reference>(true);

                std::shared_ptr<Expression> value = nullptr;
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

                std::string data_name = "data_" + value->get_expression_code();
                instructions.insert(it, std::make_shared<Declaration>("Ov_Data", std::make_shared<Symbol>(data_name)));
                instructions.insert(it, std::make_shared<Affectation>(
                    std::make_shared<Property>(
                        std::make_shared<Symbol>(data_name),
                        type == Bool ? "b" : type == Int ? "i" : type == Float ? "f" : "ptr",
                        false
                    ),
                    value
                ));

                instructions.insert(it, std::make_shared<Affectation>(
                    r,
                    std::make_shared<FunctionCall>(FunctionCall(
                        std::make_shared<Symbol>("Ov_Reference_new_data"),
                        {
                            std::make_shared<FunctionCall>(FunctionCall(
                                std::make_shared<Symbol>("Ov_UnknownData_from_data"),
                                {
                                    std::make_shared<Referencing>(std::make_shared<Symbol>("Ov_VirtualTable_" + type->name.get())),
                                    std::make_shared<Symbol>(data_name)
                                }
                            ))
                        }
                    ))
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

            instructions.insert(it, list);

            instructions.insert(it, std::make_shared<Affectation>(
                r,
                std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Reference_new_tuple"),
                    {
                        list,
                        std::make_shared<Value>(static_cast<long>(list->objects.size())),
                        std::make_shared<Referencing>(std::make_shared<Symbol>("Ov_VirtualTable_Object"))
                    }
                ))
            ));

            for (auto ref : tmp) {
                instructions.insert(it, std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("Ov_Reference_free"),
                    {
                        ref
                    }
                )));
            }

            return r;
        } else return nullptr;
    }

}
