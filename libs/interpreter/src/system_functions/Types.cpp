#include <memory>
#include <string>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>

#include <ouverium/types.h>

#include "SystemFunction.hpp"


namespace Interpreter::SystemFunctions::Types {

    auto const constructor_args = std::make_shared<Parser::Symbol>("a");

    Reference char_constructor(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (a.is<char>())
            return a;
        else
            throw FunctionArgumentsError();
    }

    Reference float_constructor(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            return Data((OV_FLOAT) *a_int);
        } else if (a.is<OV_FLOAT>()) {
            return a;
        }
        throw FunctionArgumentsError();
    }

    Reference int_constructor(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (a.is<OV_INT>()) {
            return a;
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            return Data((OV_INT) *a_float);
        }
        throw FunctionArgumentsError();
    }

    Reference bool_constructor(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (a.is<bool>())
            return a;
        else
            throw FunctionArgumentsError();
    }

    Reference array_constructor(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (auto const* obj = get_if<ObjectPtr>(&a); obj && (*obj)->array.capacity() > 0)
            return a;
        else
            throw FunctionArgumentsError();
    }

    auto const tuple_constructor_args = std::make_shared<Parser::FunctionCall>(
        std::make_shared<Parser::Symbol>("function"),
        std::make_shared<Parser::Tuple>()
    );
    Reference tuple_constructor(FunctionContext& context) {
        try {
            auto function = std::get<CustomFunction>(context["function"].to_data(context).get<ObjectPtr>()->functions.front())->body;

            if (std::dynamic_pointer_cast<Parser::Tuple>(function)) {
                return Interpreter::call_function(context.get_parent(), nullptr, context["function"], std::make_shared<Parser::Tuple>());
            } else {
                auto ptr = GC::new_object();
                auto reference = Interpreter::call_function(context.get_parent(), nullptr, context["function"], std::make_shared<Parser::Tuple>());
                ptr->array.push_back(reference.to_data(context.get_parent()));
                return Data(ptr);
            }
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference function_constructor(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (auto const* obj = get_if<ObjectPtr>(&a); obj && !(*obj)->functions.empty())
            return a;
        else
            throw FunctionArgumentsError();
    }

    auto const reference_constructor_args = std::make_shared<Parser::FunctionCall>(
        std::make_shared<Parser::Symbol>("function"),
        std::make_shared<Parser::Tuple>()
    );
    Reference reference_constructor(FunctionContext& context) {
        try {
            auto function = std::get<CustomFunction>(context["function"].to_data(context).get<ObjectPtr>()->functions.front())->body;
            auto& parent = context.get_parent();

            auto wrap = [](Reference const& reference) {
                ObjectPtr object = GC::new_object();
                auto function_definition = std::make_shared<Parser::FunctionDefinition>();
                function_definition->parameters = std::make_shared<Parser::Tuple>();
                function_definition->body = std::make_shared<Parser::Symbol>("#cached");
                object->functions.emplace_front(CustomFunction{ function_definition });
                object->functions.back().extern_symbols.emplace("#cached", reference);

                auto ptr = GC::new_object();
                ptr->properties["#get_reference"] = Data(object);
                return Data(ptr);
            };

            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(function)) {
                auto ptr = GC::new_object();
                ptr->array.reserve(tuple->objects.size());

                for (auto const& e : tuple->objects) {
                    auto reference = Interpreter::execute(parent, e);
                    ptr->array.push_back(wrap(reference));
                }

                return Data(ptr);
            } else {
                auto reference = Interpreter::call_function(context.get_parent(), nullptr, context["function"], std::make_shared<Parser::Tuple>());
                return wrap(reference);
            }
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }


    Reference float_parse(FunctionContext& context) {
        try {
            auto a = context["a"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(static_cast<OV_FLOAT>(std::stod(a)));
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference int_parse(FunctionContext& context) {
        try {
            auto a = context["a"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(static_cast<OV_INT>(std::stoi(a)));
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }


    bool check_type(Context& context, Data const& data, Data const& type) {
        if (type == context["Char"].to_data(context)) return data.is<char>();
        else if (type == context["Float"].to_data(context)) return data.is<OV_FLOAT>();
        else if (type == context["Int"].to_data(context)) return data.is<OV_INT>();
        else if (type == context["Bool"].to_data(context)) return data.is<bool>();
        else if (type == context["Array"].to_data(context)) {
            if (auto const* obj = get_if<ObjectPtr>(&data))
                return (*obj)->array.capacity() > 0;
            else
                return false;
        } else if (type == context["Function"].to_data(context)) {
            if (auto const* obj = get_if<ObjectPtr>(&data))
                return !(*obj)->functions.empty();
            else
                return false;
        } else throw FunctionArgumentsError();
    }

    auto const is_type_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("data"),
            std::make_shared<Parser::Symbol>("type")
        }
    ));
    Reference is_type(FunctionContext& context) {
        auto data = context["data"].to_data(context);
        auto type = context["type"].to_data(context);

        return Data(check_type(context, data, type));
    }

    void init(GlobalContext& context) {
        add_function(context["Char"], constructor_args, char_constructor);
        add_function(context["Float"], constructor_args, float_constructor);
        add_function(context["Int"], constructor_args, int_constructor);
        add_function(context["Bool"], constructor_args, bool_constructor);
        add_function(context["Array"], constructor_args, array_constructor);
        add_function(context["Tuple"], tuple_constructor_args, tuple_constructor);
        add_function(context["Function"], constructor_args, function_constructor);
        add_function(context["Reference"], reference_constructor_args, reference_constructor);

        add_function(Data(get_object(context["Float"])).get_property("parse"), constructor_args, float_parse);
        add_function(Data(get_object(context["Int"])).get_property("parse"), constructor_args, int_parse);

        Function f = SystemFunction{ .parameters = is_type_args, .pointer = is_type };
        f.extern_symbols.emplace("Char", context["Char"]);
        f.extern_symbols.emplace("Float", context["Float"]);
        f.extern_symbols.emplace("Int", context["Int"]);
        f.extern_symbols.emplace("Bool", context["Bool"]);
        f.extern_symbols.emplace("Array", context["Array"]);
        f.extern_symbols.emplace("Function", context["Function"]);
        get_object(context["~"])->functions.push_front(f);
    }

}
