#include <cstddef>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>

#include <ouverium/types.h>

#include "SystemFunction.hpp"


namespace Interpreter::SystemFunctions::Base {

    auto const getter_args = std::make_shared<Parser::Symbol>("var");
    Reference getter(FunctionContext& context) {
        auto const& var = context["var"];

        Data const& data = var.get_data();
        if (data != Data{}) {
            if (auto const* obj = get_if<ObjectPtr>(&data)) {
                auto const& properties = (*obj)->properties;
                auto it = properties.find("#get_reference");
                if (it != properties.end()) {
                    return call_function(context.get_parent(), nullptr, it->second, std::make_shared<Parser::Tuple>());
                }
            }

            return data;
        } else
            throw FunctionArgumentsError();
    }
    auto const function_getter_args = std::make_shared<Parser::Symbol>("function");
    Reference function_getter(FunctionContext& context) {
        return context["function"];
    }

    Reference defined(FunctionContext& context) {
        auto const& var = context["var"];

        Data const& data = var.get_data();

        if (data == Data{}) {
            return Data(false);
        } else {
            if (auto const* obj = get_if<ObjectPtr>(&data)) {
                auto const& properties = (*obj)->properties;
                auto it = properties.find("#get_reference");
                if (it != properties.end()) {
                    return call_function(context.get_parent(), nullptr, context.get_global()["defined"], call_function(context.get_parent(), nullptr, it->second, std::make_shared<Parser::Tuple>()));
                }
            }
        }

        return Data(data != Data{});
    }

    auto const setter_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("var"),
            std::make_shared<Parser::Symbol>("data")
        }
    ));
    Reference setter(FunctionContext& context) {
        auto const& var = context["var"];
        auto data = context["data"].to_data(context);

        Data const& var_data = var.get_data();
        if (var_data != Data{}) {
            if (auto const* var_object = get_if<ObjectPtr>(&var_data)) {
                auto const& properties = (*var_object)->properties;
                auto it = properties.find("#get_reference");
                if (it != properties.end()) {
                    return Interpreter::set(context, call_function(context.get_parent(), nullptr, it->second, std::make_shared<Parser::Tuple>()), data);
                }

                if (properties.contains("#reference_wrapper")) {
                    if (auto const* object = get_if<ObjectPtr>(&data)) {
                        if ((*var_object)->array.size() == (*object)->array.size()) {
                            for (size_t i = 0; i < (*var_object)->array.size(); ++i)
                                Interpreter::set(context, (*var_object)->array[i], (*object)->array[i]);
                        } else throw Interpreter::FunctionArgumentsError();

                        for (auto const& [name, d] : (*object)->properties)
                            Interpreter::set(context, data.get_property(name), d);

                        return var;
                    } else
                        throw Interpreter::FunctionArgumentsError();
                }
            }
        }

        var.data() = data;
        return var;
    }


    auto const separator_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::Symbol>("b")
        }
    ));
    Reference separator(FunctionContext& context) {
        return context["b"];
    }

    auto const if_statement_args = std::make_shared<Parser::FunctionCall>(
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
                            if (IndirectReference(else_s) == context["else"] && i + 1 < tuple->objects.size()) {
                                auto s = Interpreter::execute(parent, tuple->objects[i + 1]);
                                if (IndirectReference(s) == context["if"] && i + 3 < tuple->objects.size()) {
                                    if (Interpreter::execute(parent, tuple->objects[i + 2]).to_data(context).get<bool>())
                                        return Interpreter::execute(parent, tuple->objects[i + 3]);
                                    else i += 4;
                                } else return s;
                            } else throw Interpreter::FunctionArgumentsError();
                        }
                        return {};
                    }
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const while_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
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

    auto const for_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
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
            auto const& variable = context["variable"];
            auto begin = context["begin"].to_data(context).get<OV_INT>();
            auto end = context["end"].to_data(context).get<OV_INT>();
            auto const& block = context["block"];

            for (OV_INT i = begin; i < end; ++i) {
                Interpreter::set(context, variable, Data(i));
                Interpreter::call_function(context.get_parent(), nullptr, block, std::make_shared<Parser::Tuple>());
            }
            return {};
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const for_step_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
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

            auto const& variable = context["variable"];
            auto begin = context["begin"].to_data(context).get<OV_INT>();
            auto end = context["end"].to_data(context).get<OV_INT>();
            auto s = context["s"].to_data(context).get<OV_INT>();
            auto const& block = context["block"];

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

            return Data(GC::new_object());
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const try_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
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
        auto const& try_block = context["try_block"];
        auto const& catch_function = context["catch_function"];

        try {
            return Interpreter::call_function(context.get_parent(), nullptr, try_block, std::make_shared<Parser::Tuple>());
        } catch (Exception& ex) {
            auto r = Interpreter::try_call_function(context.get_parent(), nullptr, catch_function, ex.reference);

            if (auto* reference = std::get_if<Reference>(&r))
                return *reference;
            else
                throw std::move(ex);
        }
    }

    auto const throw_statement_args = std::make_shared<Parser::Symbol>("throw_expression");
    Reference throw_statement(FunctionContext& context) {
        throw Exception(
            context,
            context.caller,
            context["throw_expression"]
        );
    }

    Reference copy(Data const& data) {
        try {
            return Data(GC::new_object(*data.get<ObjectPtr>()));
        } catch (Data::BadAccess const&) {
            return data;
        }
    }

    Reference copy_pointer(Data const& data) {
        return data;
    }

    auto const define_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("var"),
            std::make_shared<Parser::Symbol>("data")
        }
    ));
    Reference define(FunctionContext& context) {
        auto const& var = context["var"];
        auto data = context["data"].to_data(context);

        Data const& var_data = var.get_data();
        if (var_data != Data{}) {
            if (auto const* var_object = get_if<ObjectPtr>(&var_data)) {
                auto it = (*var_object)->properties.find("#get_reference");
                if (it != (*var_object)->properties.end()) {
                    return call_function(context.get_parent(), nullptr, context.get_global()["define"], std::vector<Arguments>{call_function(context.get_parent(), nullptr, it->second, std::make_shared<Parser::Tuple>()), data});
                }
            }
        }

        if (var_data == Data{})
            return Interpreter::set(context, var, data);
        else
            throw FunctionArgumentsError();
    }

    Reference inject(FunctionContext& context, Data const& var, Data const& data) {
        if (auto const* object = get_if<ObjectPtr>(&data)) {
            for (auto const& [name, d] : (*object)->properties) {
                Interpreter::set(context, PropertyReference{ .parent = var, .name = name }, d);
            }
        }

        return {};
    }

    auto const function_definition_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("object"),
            std::make_shared<Parser::Symbol>("functions")
        }
    ));
    Reference function_definition(FunctionContext& context) {
        try {
            auto object = context["object"];
            auto functions = context["functions"].to_data(context).get<ObjectPtr>()->functions;

            auto& data = object.data();
            if (data == Data{})
                data = GC::new_object();
            auto obj = object.to_data(context).get<ObjectPtr>();

            for (auto const& function : std::ranges::reverse_view(functions))
                obj->functions.push_front(function);

            return object;
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const function_add_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("object"),
            std::make_shared<Parser::Symbol>("functions")
        }
    ));
    Reference function_add(FunctionContext& context) {
        try {
            auto object = context["object"];
            auto functions = context["functions"].to_data(context).get<ObjectPtr>()->functions;

            auto& data = object.data();
            if (data == Data{})
                data = GC::new_object();
            auto obj = object.to_data(context).get<ObjectPtr>();

            for (auto const& function : functions)
                obj->functions.push_back(function);

            return object;
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    bool eq(Data const& a, Data const& b) {
        if (auto const* a_object = get_if<ObjectPtr>(&a)) {
            if (auto const* b_object = get_if<ObjectPtr>(&b))
                return (*a_object)->properties == (*b_object)->properties
                && (*a_object)->functions == (*b_object)->functions
                && (*a_object)->array == (*b_object)->array;
            else return false;
        } else if (auto const* a_char = get_if<char>(&a)) {
            if (auto const* b_char = get_if<char>(&b)) return *a_char == *b_char;
            else return false;
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_float = get_if<OV_FLOAT>(&b)) return *a_float == *b_float;
            else return false;
        } else if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b)) return *a_int == *b_int;
            else return false;
        } else if (auto const* a_bool = get_if<bool>(&a)) {
            if (auto const* b_bool = get_if<bool>(&b)) return *a_bool == *b_bool;
            else return false;
        } else throw FunctionArgumentsError();
    }

    Reference equals(Data const& a, Data const& b) {
        return Data(eq(a, b));
    }

    Reference not_equals(Data const& a, Data const& b) {
        return Data(!eq(a, b));
    }

    Reference check_pointers(Data const& a, Data const& b) {
        return Data(a == b);
    }

    Reference not_check_pointers(Data const& a, Data const& b) {
        return Data(a != b);
    }

    Reference string_from(FunctionContext& context, Data const& data) {
        std::ostringstream os;

        if (auto const* object = get_if<ObjectPtr>(&data)) {
            try {
                if ((*object)->array.capacity() > 0 && (*object)->properties.empty())
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
        } else if (auto const* c = get_if<char>(&data))
            os << *c;
        else if (auto const* f = get_if<OV_FLOAT>(&data))
            os << *f;
        else if (auto const* i = get_if<OV_INT>(&data))
            os << *i;
        else if (auto const* b = get_if<bool>(&data))
            os << std::boolalpha << *b;

        return Data(GC::new_object(os.str()));
    }

    auto const print_args = std::make_shared<Parser::Symbol>("data");
    Reference print(FunctionContext& context) {
        auto const& data = context["data"];

        auto str = Interpreter::string_from(context, data);
        std::cout << str << std::endl;

        return {};
    }

    auto const scan_args = std::make_shared<Parser::Tuple>();
    Reference scan() {
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
        Function if_s = SystemFunction{ .parameters = if_statement_args, .pointer = if_statement };
        if_s.extern_symbols.emplace("if", context["if"]);
        if_s.extern_symbols.emplace("else", context["else"]);
        get_object(context["if"])->functions.push_front(if_s);

        add_function(context["while"], while_statement_args, while_statement);

        get_object(context["from"]);
        get_object(context["to"]);
        Function for_s = SystemFunction{ .parameters = for_statement_args, .pointer = for_statement };
        for_s.extern_symbols.emplace("from", context["from"]);
        for_s.extern_symbols.emplace("to", context["to"]);
        get_object(context["for"])->functions.push_front(for_s);

        get_object(context["step"]);
        Function for_step_s = SystemFunction{ .parameters = for_step_statement_args, .pointer = for_step_statement };
        for_step_s.extern_symbols.emplace("from", context["from"]);
        for_step_s.extern_symbols.emplace("to", context["to"]);
        for_step_s.extern_symbols.emplace("step", context["step"]);
        get_object(context["for"])->functions.push_front(for_step_s);

        get_object(context["catch"]);
        Function try_s = SystemFunction{ .parameters = try_statement_args, .pointer = try_statement };
        try_s.extern_symbols.emplace("catch", context["catch"]);
        get_object(context["try"])->functions.push_front(try_s);
        add_function(context["throw"], throw_statement_args, throw_statement);

        add_function(context["$"], copy);
        add_function(context["$=="], copy_pointer);

        add_function(context["="], define_args, define);
        add_function(context[":<-"], inject);
        add_function(context[":"], function_definition_args, function_definition);
        add_function(context["|"], function_add_args, function_add);

        add_function(context["=="], equals);
        add_function(context["!="], not_equals);
        add_function(context["==="], check_pointers);
        add_function(context["!=="], not_check_pointers);

        add_function(context["string_from"], string_from);
        add_function(context["print"], print_args, print);
        add_function(context["scan"], scan);
    }

}
