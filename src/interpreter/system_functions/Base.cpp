#include <iostream>

#include "Base.hpp"


namespace Interpreter::SystemFunctions {

    namespace Base {

        auto getter_args = std::make_shared<Parser::Symbol>("var");
        Reference getter(FunctionContext& context) {
            auto data = context["var"].get_raw();
            if (data != Data{})
                return data;
            else
                throw FunctionArgumentsError();
        }

        Reference defined(FunctionContext& context) {
            return context["var"].get_raw() != Data{};
        }

        Reference assignation(Context& context, Reference const& var, Data const& d) {
            if (std::get_if<Data>(&var)) return d;
            else if (auto symbol_reference = std::get_if<SymbolReference>(&var)) symbol_reference->get() = d;
            else if (auto property_reference = std::get_if<PropertyReference>(&var)) static_cast<Data&>(*property_reference) = d;
            else if (auto array_reference = std::get_if<ArrayReference>(&var)) static_cast<Data&>(*array_reference) = d;
            else if (auto tuple_reference = std::get_if<TupleReference>(&var)) {
                try {
                    auto object = d.get<Object*>();
                    if (tuple_reference->size() == object->array.size()) {
                        for (size_t i = 0; i < tuple_reference->size(); ++i)
                            assignation(context, (*tuple_reference)[i], object->array[i]);
                    } else throw Interpreter::FunctionArgumentsError();
                } catch (Data::BadAccess const&) {
                    throw Interpreter::FunctionArgumentsError();
                }
            }
            return var;
        }

        auto setter_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("var"),
                    std::make_shared<Parser::Tuple>()
                ),
                std::make_shared<Parser::Symbol>("data")
            }
        ));
        Reference setter(FunctionContext& context) {
            auto var = Interpreter::call_function(context.get_parent(), context.expression, context["var"], std::make_shared<Parser::Tuple>());
            auto data = context["data"].to_data(context);

            return assignation(context, var, data);
        }


        auto separator_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("a"),
                std::make_shared<Parser::Symbol>("b")
            }
        ));
        Reference separator(FunctionContext& context) {
            return Reference(context["b"]);
        }

        auto if_statement_args = std::make_shared<Parser::FunctionCall>(
            std::make_shared<Parser::Symbol>("function"),
            std::make_shared<Parser::Tuple>()
        );
        Reference if_statement(FunctionContext& context) {
            try {
                auto function = std::get<CustomFunction>(context["function"].to_data(context).get<Object*>()->functions.front())->body;

                if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(function)) {
                    if (tuple->objects.size() >= 2) {
                        auto& parent = context.get_parent();

                        if (Interpreter::execute(parent, tuple->objects[0]).to_data(context).get<bool>())
                            return Interpreter::execute(parent, tuple->objects[1]);
                        else {
                            size_t i = 2;
                            while (i < tuple->objects.size()) {
                                auto else_s = Interpreter::execute(parent, tuple->objects[i]).to_data(context);
                                if (else_s == context["else"].to_data(context) && i + 1 < tuple->objects.size()) {
                                    auto s = Interpreter::execute(parent, tuple->objects[i + 1]);
                                    if (s.to_data(context) == context["if"].to_data(context) && i + 3 < tuple->objects.size()) {
                                        if (Interpreter::execute(parent, tuple->objects[i + 2]).to_data(context).get<bool>())
                                            return Interpreter::execute(parent, tuple->objects[i + 3]);
                                        else i += 4;
                                    } else return s;
                                } else throw Interpreter::FunctionArgumentsError();
                            }
                            return Reference();
                        }
                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto while_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("condition"),
                    std::make_shared<Parser::Tuple>()
                ),
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("block"),
                    std::make_shared<Parser::Tuple>()
                )
            }
        ));
        Reference while_statement(FunctionContext& context) {
            try {
                auto& parent = context.get_parent();
                Reference result;

                auto condition = context["condition"];
                auto block = context["block"];
                while (true) {
                    auto c = Interpreter::call_function(parent, parent.expression, condition, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>();
                    if (c) {
                        result = Interpreter::call_function(parent, parent.expression, block, std::make_shared<Parser::Tuple>());
                    } else break;
                }

                return result;
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto for_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("variable"),
                std::make_shared<Parser::Symbol>("from"),
                std::make_shared<Parser::Symbol>("begin"),
                std::make_shared<Parser::Symbol>("to"),
                std::make_shared<Parser::Symbol>("end"),
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("block"),
                    std::make_shared<Parser::Tuple>()
                )
            }
        ));
        Reference for_statement(FunctionContext& context) {
            try {
                auto variable = context["variable"];
                auto begin = context["begin"].to_data(context).get<OV_INT>();
                auto end = context["end"].to_data(context).get<OV_INT>();
                auto block = context["block"];

                for (OV_INT i = begin; i < end; ++i) {
                    Interpreter::set(context, variable, Data(i));
                    Interpreter::call_function(context.get_parent(), context.expression, block, std::make_shared<Parser::Tuple>());
                }
                return Reference();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto for_step_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("variable"),
                std::make_shared<Parser::Symbol>("from"),
                std::make_shared<Parser::Symbol>("begin"),
                std::make_shared<Parser::Symbol>("to"),
                std::make_shared<Parser::Symbol>("end"),
                std::make_shared<Parser::Symbol>("step"),
                std::make_shared<Parser::Symbol>("s"),
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("block"),
                    std::make_shared<Parser::Tuple>()
                )
            }
        ));
        Reference for_step_statement(FunctionContext& context) {
            try {
                auto& parent = context.get_parent();

                auto variable = context["variable"];
                auto begin = context["begin"].to_data(context).get<OV_INT>();
                auto end = context["end"].to_data(context).get<OV_INT>();
                auto s = context["s"].to_data(context).get<OV_INT>();
                auto block = context["block"];

                if (s > 0) {
                    for (OV_INT i = begin; i < end; i += s) {
                        Interpreter::set(context, variable, Data(i));
                        Interpreter::call_function(parent, parent.expression, block, std::make_shared<Parser::Tuple>());
                    }
                } else if (s < 0) {
                    for (OV_INT i = begin; i > end; i += s) {
                        Interpreter::set(context, variable, Data(i));
                        Interpreter::call_function(parent, parent.expression, block, std::make_shared<Parser::Tuple>());
                    }
                } else throw Interpreter::FunctionArgumentsError();
                return Reference(Data(context.new_object()));
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto try_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("try_block"),
                    std::make_shared<Parser::Tuple>()
                ),
                std::make_shared<Parser::Symbol>("catch"),
                std::make_shared<Parser::Symbol>("catch_function")
            }
        ));
        Reference try_statement(FunctionContext& context) {
            auto try_block = context["try_block"];
            auto catch_function = context["catch_function"];

            try {
                return Interpreter::call_function(context.get_parent(), context.expression, try_block, std::make_shared<Parser::Tuple>());
            } catch (Exception const& ex) {
                try {
                    return Interpreter::call_function(context.get_parent(), context.expression, catch_function, ex.reference);
                } catch (Interpreter::Exception const&) {
                    throw ex;
                }
            }
        }

        auto throw_statement_args = std::make_shared<Parser::Symbol>("throw_expression");
        Reference throw_statement(FunctionContext& context) {
            throw Exception(
                context,
                context["throw_expression"],
                context.get_parent().expression
            );
        }

        auto copy_args = std::make_shared<Parser::Symbol>("data");
        Reference copy(FunctionContext& context) {
            auto data = context["data"].to_data(context);

            try {
                return Data(context.new_object(*data.get<Object*>()));
            } catch (Data::BadAccess const&) {
                return data;
            }
        }

        Reference copy_pointer(FunctionContext& context) {
            return Reference(context["data"].to_data(context));
        }

        auto define_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("var"),
                    std::make_shared<Parser::Tuple>()
                ),
                std::make_shared<Parser::Symbol>("data")
            }
        ));
        Reference define(FunctionContext& context) {
            auto var = Interpreter::call_function(context.get_parent(), context.expression, context["var"], std::make_shared<Parser::Tuple>());
            auto data = context["data"].to_data(context);

            std::function<bool(Reference const&)> iterate = [&iterate, &context](Reference const& reference) -> bool {
                if (auto tuple = std::get_if<TupleReference>(&reference)) {
                    for (auto const& r : *tuple)
                        if (iterate(r))
                            return true;

                    return false;
                } else {
                    return std::visit([](auto const& arg) -> Data const& {
                        return arg;
                    }, reference.to_indirect_reference(context)) != Data{};
                }
            };

            if (!iterate(var))
                return Interpreter::set(context, var, data);
            else
                throw FunctionArgumentsError();
        }

        auto function_definition_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("var"),
                std::make_shared<Parser::Symbol>("data")
            }
        ));
        Reference function_definition(FunctionContext& context) {
            try {
                auto var = context["var"].to_data(context).get<Object*>();
                auto functions = context["data"].to_data(context).get<Object*>()->functions;

                for (auto it = functions.rbegin(); it != functions.rend(); it++)
                    var->functions.push_front(*it);

                return context["var"];
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        bool eq(Data a, Data b) {
            if (auto a_object = get_if<Object*>(&a)) {
                if (auto b_object = get_if<Object*>(&b))
                    return (*a_object)->properties == (*b_object)->properties && (*a_object)->array == (*b_object)->array;
                else return false;
            } else if (auto a_char = get_if<char>(&a)) {
                if (auto b_char = get_if<char>(&b)) return *a_char == *b_char;
                else return false;
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_float = get_if<OV_FLOAT>(&b)) return *a_float == *b_float;
                else return false;
            } else if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b)) return *a_int == *b_int;
                else return false;
            } else if (auto a_bool = get_if<bool>(&a)) {
                if (auto b_bool = get_if<bool>(&b)) return *a_bool == *b_bool;
                else return false;
            } else throw FunctionArgumentsError();
        }

        auto equals_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("a"),
                std::make_shared<Parser::Symbol>("b")
            }
        ));
        Reference equals(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            return Reference(Data(eq(a, b)));
        }

        Reference not_equals(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            return Reference(Data(!eq(a, b)));
        }

        Reference check_pointers(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            return Reference(Data(a == b));
        }

        Reference not_check_pointers(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            return Reference(Data(a != b));
        }

        auto string_from_args = std::make_shared<Parser::Symbol>("data");
        Reference string_from(FunctionContext& context) {
            auto data = context["data"].to_data(context);

            std::ostringstream os;

            if (auto object = get_if<Object*>(&data)) {
                if (object && *object) {
                    try {
                        if ((*object)->array.capacity() > 0)
                            os << (*object)->to_string();
                        else
                            throw Data::BadAccess();
                    } catch (Data::BadAccess const&) {
                        bool prev = false;
                        os << "(";
                        for (auto const& [key, value] : (*object)->properties) if (value != Data{}) {
                            if (prev)
                                os << ", ";
                            prev = true;
                            os << key << ": " << Interpreter::string_from(context, value);
                        }
                        for (auto d : (*object)->array) {
                            if (prev)
                                os << ", ";
                            prev = true;
                            os << Interpreter::string_from(context, d);
                        }
                        os << ")";
                    }
                }
            } else if (auto c = get_if<char>(&data))
                os << *c;
            else if (auto f = get_if<OV_FLOAT>(&data))
                os << *f;
            else if (auto i = get_if<OV_INT>(&data))
                os << *i;
            else if (auto b = get_if<bool>(&data))
                os << (*b ? "true" : "false");

            return Data(context.new_object(os.str()));
        }

        auto print_args = std::make_shared<Parser::Symbol>("data");
        Reference print(FunctionContext& context) {
            auto data = context["data"];

            auto str = Interpreter::string_from(context, data);
            std::cout << str << std::endl;

            return Reference();
        }

        auto scan_args = std::make_shared<Parser::Tuple>();
        Reference scan(FunctionContext& context) {
            std::string str;
            getline(std::cin, str);

            return Data(context.new_object(str));
        }


        void init(GlobalContext& context) {
            insert(context, "getter")->functions.push_front(SystemFunction{ getter_args, getter });

            insert(context, "defined")->functions.push_front(SystemFunction{ getter_args, defined });

            insert(context, "setter")->functions.push_front(SystemFunction{ setter_args, setter });
            set(context, context[":="], context["setter"]);


            insert(context, "NotAFunction");
            insert(context, "IncorrectFunctionArguments");
            insert(context, "ParserException");
            insert(context, "RecursionLimitExceeded");


            insert(context, ";")->functions.push_front(SystemFunction{ separator_args, separator });

            insert(context, "if");
            insert(context, "else");
            Function if_s = SystemFunction{ if_statement_args, if_statement };
            if_s.extern_symbols.emplace("if", context["if"]);
            if_s.extern_symbols.emplace("else", context["else"]);
            insert(context, "if")->functions.push_front(if_s);

            insert(context, "while")->functions.push_front(SystemFunction{ while_statement_args, while_statement });

            insert(context, "from");
            insert(context, "to");
            Function for_s = SystemFunction{ for_statement_args, for_statement };
            for_s.extern_symbols.emplace("from", context["from"]);
            for_s.extern_symbols.emplace("to", context["to"]);
            insert(context, "for")->functions.push_front(for_s);

            insert(context, "step");
            Function for_step_s = SystemFunction{ for_step_statement_args, for_step_statement };
            for_step_s.extern_symbols.emplace("from", context["from"]);
            for_step_s.extern_symbols.emplace("to", context["to"]);
            for_step_s.extern_symbols.emplace("step", context["step"]);
            insert(context, "for")->functions.push_front(for_step_s);

            insert(context, "catch");
            Function try_s = SystemFunction{ try_statement_args, try_statement };
            try_s.extern_symbols.emplace("catch", context["catch"]);
            insert(context, "try")->functions.push_front(try_s);
            insert(context, "throw")->functions.push_front(SystemFunction{ throw_statement_args, throw_statement });

            insert(context, "$")->functions.push_front(SystemFunction{ copy_args, copy });
            insert(context, "$==")->functions.push_front(SystemFunction{ copy_args, copy_pointer });

            insert(context, "=")->functions.push_front(SystemFunction{ define_args, define });
            insert(context, ":")->functions.push_front(SystemFunction{ function_definition_args, function_definition });

            insert(context, "==")->functions.push_front(SystemFunction{ equals_args, equals });
            insert(context, "!=")->functions.push_front(SystemFunction{ equals_args, not_equals });
            insert(context, "===")->functions.push_front(SystemFunction{ equals_args, check_pointers });
            insert(context, "!==")->functions.push_front(SystemFunction{ equals_args, not_check_pointers });

            insert(context, "string_from")->functions.push_front(SystemFunction{ string_from_args, string_from });
            insert(context, "print")->functions.push_front(SystemFunction{ print_args, print });
            insert(context, "scan")->functions.push_front(SystemFunction{ scan_args, scan });
        }

    }

}
