#include <iostream>

#include "SystemFunction.hpp"


namespace Interpreter::SystemFunctions {

    namespace Base {

        auto getter_args = std::make_shared<Parser::Symbol>("var");
        Reference getter(FunctionContext& context) {
            auto var = context["var"];

            Data data = var.get_data();
            if (data != Data{})
                return data;
            else
                throw FunctionArgumentsError();
        }
        auto function_getter_args = std::make_shared<Parser::Symbol>("function");
        Reference function_getter(FunctionContext& context) {
            return context["function"];
        }

        Reference defined(FunctionContext& context) {
            auto var = context["var"];

            return Data(var.get_data() != Data{});
        }

        Reference assignation(Context& context, Reference const& var, Data const& d) {
            if (std::get_if<Data>(&var)) return d;
            else if (auto symbol_reference = std::get_if<SymbolReference>(&var)) symbol_reference->it->first = d;
            else if (auto property_reference = std::get_if<PropertyReference>(&var)) {
                auto parent = property_reference->parent;
                if (auto obj = get_if<ObjectPtr>(&parent))
                    (*obj)->properties[property_reference->name] = d;
            } else if (auto array_reference = std::get_if<ArrayReference>(&var)) {
                auto array = array_reference->array;
                if (auto obj = get_if<ObjectPtr>(&array))
                    (*obj)->array[array_reference->i] = d;
            } else if (auto tuple_reference = std::get_if<TupleReference>(&var)) {
                try {
                    auto object = d.get<ObjectPtr>();
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
            auto var = Interpreter::call_function(context.get_parent(), nullptr, context["var"], std::make_shared<Parser::Tuple>());
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
                auto function = std::get<CustomFunction>(context["function"].to_data(context).get<ObjectPtr>()->functions.front())->body;

                if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(function)) {
                    if (tuple->objects.size() >= 2) {
                        auto& parent = context.get_parent();

                        if (Interpreter::execute(parent, tuple->objects[0]).to_data(context).get<bool>())
                            return Interpreter::execute(parent, tuple->objects[1]);
                        else {
                            size_t i = 2;
                            while (i < tuple->objects.size()) {
                                auto else_s = Interpreter::execute(parent, tuple->objects[i]);
                                if (else_s.to_indirect_reference(context) == context["else"] && i + 1 < tuple->objects.size()) {
                                    auto s = Interpreter::execute(parent, tuple->objects[i + 1]);
                                    if (s.to_indirect_reference(context) == context["if"] && i + 3 < tuple->objects.size()) {
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
                    auto c = Interpreter::call_function(parent, nullptr, condition, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>();
                    if (c) {
                        result = Interpreter::call_function(parent, nullptr, block, std::make_shared<Parser::Tuple>());
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
                    Interpreter::call_function(context.get_parent(), nullptr, block, std::make_shared<Parser::Tuple>());
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
                        Interpreter::call_function(parent, nullptr, block, std::make_shared<Parser::Tuple>());
                    }
                } else if (s < 0) {
                    for (OV_INT i = begin; i > end; i += s) {
                        Interpreter::set(context, variable, Data(i));
                        Interpreter::call_function(parent, nullptr, block, std::make_shared<Parser::Tuple>());
                    }
                } else throw Interpreter::FunctionArgumentsError();
                return Reference(Data(GC::new_object()));
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
                return Interpreter::call_function(context.get_parent(), nullptr, try_block, std::make_shared<Parser::Tuple>());
            } catch (Exception const& ex) {
                auto r = Interpreter::try_call_function(context.get_parent(), nullptr, catch_function, ex.reference);

                if (auto reference = std::get_if<Reference>(&r))
                    return *reference;
                else
                    throw ex;
            }
        }

        auto throw_statement_args = std::make_shared<Parser::Symbol>("throw_expression");
        Reference throw_statement(FunctionContext& context) {
            throw Exception(
                context,
                context.caller,
                context["throw_expression"]
            );
        }

        auto copy_args = std::make_shared<Parser::Symbol>("data");
        Reference copy(FunctionContext& context) {
            auto data = context["data"].to_data(context);

            try {
                return Data(GC::new_object(*data.get<ObjectPtr>()));
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
            auto var = Interpreter::call_function(context.get_parent(), nullptr, context["var"], std::make_shared<Parser::Tuple>());
            auto data = context["data"].to_data(context);

            std::function<bool(Reference const&)> iterate = [&iterate, &context](Reference const& reference) -> bool {
                if (auto tuple = std::get_if<TupleReference>(&reference)) {
                    for (auto const& r : *tuple)
                        if (iterate(r))
                            return true;

                    return false;
                } else {
                    return reference.to_indirect_reference(context).get_data() != Data{};
                }
            };

            if (!iterate(var))
                return Interpreter::set(context, var, data);
            else
                throw FunctionArgumentsError();
        }

        auto function_definition_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("object"),
                std::make_shared<Parser::Symbol>("functions")
            }
        ));
        Reference function_definition(FunctionContext& context) {
            try {
                auto object = context["object"];
                auto functions = context["functions"].to_data(context).get<ObjectPtr>()->functions;

                auto& data = object.get_data();
                if (data == Data{})
                    data = GC::new_object();
                auto obj = object.to_data(context).get<ObjectPtr>();

                for (auto it = functions.rbegin(); it != functions.rend(); it++)
                    obj->functions.push_front(*it);

                return object;
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto function_add_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("object"),
                std::make_shared<Parser::Symbol>("functions")
            }
        ));
        Reference function_add(FunctionContext& context) {
            try {
                auto object = context["object"];
                auto functions = context["functions"].to_data(context).get<ObjectPtr>()->functions;

                auto& data = object.get_data();
                if (data == Data{})
                    data = GC::new_object();
                auto obj = object.to_data(context).get<ObjectPtr>();

                for (auto it = functions.begin(); it != functions.end(); it++)
                    obj->functions.push_back(*it);

                return object;
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        bool eq(Data a, Data b) {
            if (auto a_object = get_if<ObjectPtr>(&a)) {
                if (auto b_object = get_if<ObjectPtr>(&b))
                    return (*a_object)->properties == (*b_object)->properties
                    && (*a_object)->functions == (*b_object)->functions
                    && (*a_object)->array == (*b_object)->array;
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

            if (auto object = get_if<ObjectPtr>(&data)) {
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
            } else if (auto c = get_if<char>(&data))
                os << *c;
            else if (auto f = get_if<OV_FLOAT>(&data))
                os << *f;
            else if (auto i = get_if<OV_INT>(&data))
                os << *i;
            else if (auto b = get_if<bool>(&data))
                os << std::boolalpha << *b;

            return Data(GC::new_object(os.str()));
        }

        auto print_args = std::make_shared<Parser::Symbol>("data");
        Reference print(FunctionContext& context) {
            auto data = context["data"];

            auto str = Interpreter::string_from(context, data);
            std::cout << str << std::endl;

            return Reference();
        }

        auto scan_args = std::make_shared<Parser::Tuple>();
        Reference scan(FunctionContext&) {
            std::string str;
            getline(std::cin, str);

            return Data(GC::new_object(str));
        }


        void init(GlobalContext& context) {
            add_function(context["getter"], getter_args, getter);
            add_function(context["function_getter"], function_getter_args, function_getter);

            add_function(context["defined"], getter_args, defined);

            add_function(context["setter"], setter_args, setter);
            set(context, context[":="], context["setter"]);


            get_object(context["NotAFunction"]);
            get_object(context["IncorrectFunctionArguments"]);
            get_object(context["ParserException"]);
            get_object(context["RecursionLimitExceeded"]);


            add_function(context[";"], separator_args, separator);

            get_object(context["if"]);
            get_object(context["else"]);
            Function if_s = SystemFunction{ if_statement_args, if_statement };
            if_s.extern_symbols.emplace("if", context["if"]);
            if_s.extern_symbols.emplace("else", context["else"]);
            get_object(context["if"])->functions.push_front(if_s);

            add_function(context["while"], while_statement_args, while_statement);

            get_object(context["from"]);
            get_object(context["to"]);
            Function for_s = SystemFunction{ for_statement_args, for_statement };
            for_s.extern_symbols.emplace("from", context["from"]);
            for_s.extern_symbols.emplace("to", context["to"]);
            get_object(context["for"])->functions.push_front(for_s);

            get_object(context["step"]);
            Function for_step_s = SystemFunction{ for_step_statement_args, for_step_statement };
            for_step_s.extern_symbols.emplace("from", context["from"]);
            for_step_s.extern_symbols.emplace("to", context["to"]);
            for_step_s.extern_symbols.emplace("step", context["step"]);
            get_object(context["for"])->functions.push_front(for_step_s);

            get_object(context["catch"]);
            Function try_s = SystemFunction{ try_statement_args, try_statement };
            try_s.extern_symbols.emplace("catch", context["catch"]);
            get_object(context["try"])->functions.push_front(try_s);
            add_function(context["throw"], throw_statement_args, throw_statement);

            add_function(context["$"], copy_args, copy);
            add_function(context["$=="], copy_args, copy_pointer);

            add_function(context["="], define_args, define);
            add_function(context[":"], function_definition_args, function_definition);
            add_function(context["|"], function_add_args, function_add);

            add_function(context["=="], equals_args, equals);
            add_function(context["!="], equals_args, not_equals);
            add_function(context["==="], equals_args, check_pointers);
            add_function(context["!=="], equals_args, not_check_pointers);

            add_function(context["string_from"], string_from_args, string_from);
            add_function(context["print"], print_args, print);
            add_function(context["scan"], scan_args, scan);
        }

    }

}
