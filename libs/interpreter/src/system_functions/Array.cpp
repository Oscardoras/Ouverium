#include <algorithm>
#include <cstddef>
#include <memory>

#include <ouverium/types.h>

#include "SystemFunction.hpp"

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Interpreter::SystemFunctions::Array {

    Reference get_capacity(ObjectPtr const& array) {
        return Data(static_cast<OV_INT>(array->array.capacity()));
    }

    Reference set_capacity(ObjectPtr const& array, OV_INT capacity) {
        array->array.reserve(capacity);

        return Data();
    }

    Reference get_size(ObjectPtr const& array) {
        return Data(static_cast<OV_INT>(array->array.size()));
    }

    Reference set_size(ObjectPtr const& array, OV_INT size) {
        array->array.resize(size);

        return Data();
    }

    Reference get(ObjectPtr const& array, OV_INT i) {
        if (i >= 0 && i < static_cast<OV_INT>(array->array.size()))
            return Data(array).get_at(static_cast<size_t>(i));
        else throw FunctionArgumentsError();
    }

    auto const copy_data_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("from_array"),
            std::make_shared<Parser::Symbol>("from_i"),
            std::make_shared<Parser::Symbol>("to_array"),
            std::make_shared<Parser::Symbol>("to_i"),
            std::make_shared<Parser::Symbol>("n")
        }
    ));
    Reference copy_data(ObjectPtr const& from_array, OV_INT from_i, ObjectPtr const& to_array, OV_INT to_i, OV_INT n) {
        if (n < 0)
            throw FunctionArgumentsError();
        else if (n == 0)
            return Data{};
        if (from_i < 0 || from_i + static_cast<size_t>(n) > from_array->array.size())
            throw FunctionArgumentsError();
        if (to_i < 0 || to_i + static_cast<size_t>(n) > to_array->array.size())
            throw FunctionArgumentsError();

        if (from_i < to_i) {
            for (OV_INT i = n - 1; i >= 0; --i)
                to_array->array[to_i + i] = from_array->array[from_i + i];
        } else {
            for (OV_INT i = 0; i < n; ++i)
                to_array->array[to_i + i] = from_array->array[from_i + i];
        }

        return Data{};
    }

    auto const foreach_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("array"),
                std::make_shared<Parser::Tuple>()
            ),
            std::make_shared<Parser::Symbol>("function")
        }
    ));
    Reference foreach(FunctionContext& context) {
        try {
            auto array = Interpreter::call_function(context.get_parent(), nullptr, context["array"], std::make_shared<Parser::Tuple>());

            auto obj = array.to_data(context).get<ObjectPtr>();
            size_t size = obj->array.size();
            for (size_t i = 0; i < size; ++i)
                Interpreter::call_function(context.get_parent(), nullptr, context["function"], Data(obj).get_at(i));

            return Data{};
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const function_extract_args = std::make_shared<Parser::Symbol>("function");
    Reference function_extract(FunctionContext& context) {
        try {
            auto const& functions = context["function"].to_data(context).get<ObjectPtr>()->functions;

            auto object = GC::new_object();
            object->array.reserve(std::min(static_cast<size_t>(1), functions.size()));
            for (auto const& f : functions) {
                auto obj = GC::new_object();
                obj->functions.push_front(f);
                object->array.emplace_back(obj);
            }

            return Data(object);
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const function_clear_args = std::make_shared<Parser::Symbol>("function");
    Reference function_clear(FunctionContext& context) {
        try {
            auto function = context["function"].to_data(context).get<ObjectPtr>();

            function->functions.clear();

            return context["function"];
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    void init(GlobalContext& context) {
        auto array = get_object(context["Array"]);
        add_function(Data(array).get_property("get_capacity"), get_capacity);
        add_function(Data(array).get_property("set_capacity"), set_capacity);
        add_function(Data(array).get_property("get_size"), get_size);
        add_function(Data(array).get_property("set_size"), set_size);
        add_function(Data(array).get_property("get"), get);
        add_function(Data(array).get_property("copy_data"), copy_data);

        add_function(context["foreach"], foreach_args, foreach);

        auto function = get_object(context["Function"]);
        add_function(Data(function).get_property("extract"), function_extract_args, function_extract);
        add_function(Data(function).get_property("clear"), function_clear_args, function_clear);
    }

}
