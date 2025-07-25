#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

#include <ouverium/types.h>

#include "SystemFunction.hpp"

#include "../Interpreter.hpp"

#include "../../parser/Expressions.hpp"


namespace Interpreter::SystemFunctions::Math {

    auto const a = std::make_shared<Parser::Symbol>("a");
    auto const ab = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::Symbol>("b")
        }
    ));
    auto const a_b = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("b"),
                std::make_shared<Parser::Tuple>()
            )
        }
    ));

    Reference logical_not(bool a) {
        try {
            return Data(!a);
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference logical_and(FunctionContext& context) {
        try {
            auto a = context["a"].to_data(context).get<bool>();
            auto b = context["b"];

            if (a)
                return Data(Interpreter::call_function(context.get_parent(), nullptr, b, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>());
            else
                return Data(false);
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference logical_or(FunctionContext& context) {
        try {
            auto a = context["a"].to_data(context).get<bool>();
            auto b = context["b"];

            if (!a)
                return Data(Interpreter::call_function(context.get_parent(), nullptr, b, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>());
            else
                return Data(true);
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference addition(Data const& a, Data const& b) {
        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int + *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) + *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float + static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float + *b_float);
        }
        throw FunctionArgumentsError();
    }

    Reference opposite(FunctionContext& context) {
        auto a = context["a"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a))
            return Data(-*a_int);
        else if (auto const* a_float = get_if<OV_FLOAT>(&a))
            return Data(-*a_float);
        throw FunctionArgumentsError();
    }

    Reference substraction(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int - *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) - *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float - static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float - *b_float);
        }
        throw FunctionArgumentsError();
    }

    Reference multiplication(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int * *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) * *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float * static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float * *b_float);
        }
        throw FunctionArgumentsError();
    }

    Reference division(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int / *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) / *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float / static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float / *b_float);
        }
        throw FunctionArgumentsError();
    }

    Reference modulo(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a))
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int % *b_int);
        throw Interpreter::FunctionArgumentsError();
    }

    Reference strictly_inf(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int < *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) < *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float < static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float < *b_float);
        }
        if (auto const* a_char = get_if<char>(&a))
            if (auto const* b_char = get_if<char>(&b))
                return Data(*a_char < *b_char);
        throw FunctionArgumentsError();
    }

    Reference strictly_sup(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int > *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) > *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float > static_cast<OV_FLOAT>(*b_int));
            else if (const auto* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float > *b_float);
        }
        if (auto const* a_char = get_if<char>(&a))
            if (auto const* b_char = get_if<char>(&b))
                return Data(*a_char > *b_char);
        throw FunctionArgumentsError();
    }

    Reference inf_equals(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int <= *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) <= *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float <= static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float <= *b_float);
        }
        if (auto const* a_char = get_if<char>(&a))
            if (auto const* b_char = get_if<char>(&b))
                return Data(*a_char <= *b_char);
        throw FunctionArgumentsError();
    }

    Reference sup_equals(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_int >= *b_int);
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_FLOAT>(*a_int) >= *b_float);
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return Data(*a_float >= static_cast<OV_FLOAT>(*b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return Data(*a_float >= *b_float);
        }
        if (auto const* a_char = get_if<char>(&a))
            if (auto const* b_char = get_if<char>(&b))
                return Data(*a_char >= *b_char);
        throw FunctionArgumentsError();
    }

    Reference increment(FunctionContext& context) {
        auto a = context["a"];

        try {
            return set(context, a, Data(a.to_data(context).get<OV_INT>() + 1));
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference decrement(FunctionContext& context) {
        auto a = context["a"];

        try {
            return set(context, a, Data(a.to_data(context).get<OV_INT>() - 1));
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference add(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_int + *b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(static_cast<OV_FLOAT>(*a_int) + *b_float));
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_float + static_cast<OV_FLOAT>(*b_int)));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(*a_float + *b_float));
        }
        throw FunctionArgumentsError();
    }

    Reference remove(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_int - *b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(static_cast<OV_FLOAT>(*a_int) - *b_float));
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_float - static_cast<OV_FLOAT>(*b_int)));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(*a_float - *b_float));
        }
        throw FunctionArgumentsError();
    }

    Reference mutiply(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_int * *b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(static_cast<OV_FLOAT>(*a_int) * *b_float));
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_float * static_cast<OV_FLOAT>(*b_int)));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(*a_float * *b_float));
        }
        throw FunctionArgumentsError();
    }

    Reference divide(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_int / *b_int));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(static_cast<OV_FLOAT>(*a_int) / *b_float));
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b))
                return set(context, context["a"], Data(*a_float / static_cast<OV_FLOAT>(*b_int)));
            else if (auto const* b_float = get_if<OV_FLOAT>(&b))
                return set(context, context["a"], Data(*a_float / *b_float));
        }
        throw FunctionArgumentsError();
    }

    auto const for_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("array"),
            std::make_shared<Parser::Symbol>("function")
        }
    ));
    Reference forall(FunctionContext& context) {
        try {
            auto array = context["array"].to_data(context).get<ObjectPtr>();
            auto functions = context["function"];

            bool value = true;

            for (auto const& d : array->array) {
                if (!Interpreter::call_function(context.get_parent(), nullptr, functions, d).to_data(context).get<bool>()) {
                    value = false;
                    break;
                }
            }

            return Data{ value };
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference exists(FunctionContext& context) {
        try {
            auto array = context["array"];
            auto functions = context["function"];

            bool value = false;

            for (auto const& d : array.to_data(context).get<ObjectPtr>()->array) {
                if (Interpreter::call_function(context.get_parent(), nullptr, functions, d).to_data(context).get<bool>()) {
                    value = true;
                    break;
                }
            }

            return Data{ value };
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    thread_local std::mt19937 randgen(std::random_device{}());

    Reference random_0(FunctionContext& /*context*/) {
        std::uniform_real_distribution<OV_FLOAT> dis(0., 1.);
        return Data(dis(randgen));
    }

    Reference random_1(FunctionContext& context) {
        auto b = context["b"].to_data(context);

        if (auto const* b_int = get_if<OV_INT>(&b)) {
            std::uniform_int_distribution<OV_INT> dis(0, *b_int);
            return Data(dis(randgen));
        } else if (auto const* b_float = get_if<OV_FLOAT>(&b)) {
            std::uniform_real_distribution<OV_FLOAT> dis(0, *b_float);
            return Data(dis(randgen));
        }
        throw FunctionArgumentsError();
    }

    Reference random_2(FunctionContext& context) {
        auto a = context["a"].to_data(context);
        auto b = context["b"].to_data(context);

        if (auto const* a_int = get_if<OV_INT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b)) {
                std::uniform_int_distribution<OV_INT> dis(*a_int, *b_int);
                return Data(dis(randgen));
            } else if (auto const* b_float = get_if<OV_FLOAT>(&b)) {
                std::uniform_real_distribution<OV_FLOAT> dis(*a_int, *b_float);
                return Data(dis(randgen));
            }
        } else if (auto const* a_float = get_if<OV_FLOAT>(&a)) {
            if (auto const* b_int = get_if<OV_INT>(&b)) {
                std::uniform_real_distribution<OV_FLOAT> dis(*a_float, *b_int);
                return Data(dis(randgen));
            } else if (auto const* b_float = get_if<OV_FLOAT>(&b)) {
                std::uniform_real_distribution<OV_FLOAT> dis(*a_float, *b_float);
                return Data(dis(randgen));
            }
        }
        throw FunctionArgumentsError();
    }

    OV_FLOAT get_OV_FLOAT(Data const& data) {
        if (auto const* data_int = get_if<OV_INT>(&data))
            return static_cast<OV_FLOAT>(*data_int);
        else if (auto const* data_float = get_if<OV_FLOAT>(&data))
            return *data_float;
        else throw FunctionArgumentsError();
    }

    template<OV_FLOAT(*function)(OV_FLOAT)>
    Reference function1(FunctionContext& context) {
        return Data(function(get_OV_FLOAT(context["a"].to_data(context))));
    }

    template<OV_FLOAT(*function)(OV_FLOAT, OV_FLOAT)>
    Reference function2(FunctionContext& context) {
        return Data(function(get_OV_FLOAT(context["a"].to_data(context)), get_OV_FLOAT(context["b"].to_data(context))));
    }

    template<bool (*function)(OV_FLOAT)>
    Reference function_bool(FunctionContext& context) {
        return Data(function(get_OV_FLOAT(context["a"].to_data(context))));
    }

    void init(GlobalContext& context) {
        add_function(context["!"], logical_not);
        add_function(context["&"], a_b, logical_and);
        add_function(context["|"], a_b, logical_or);
        add_function(context["+"], addition);
        add_function(context["-"], a, opposite);
        add_function(context["-"], ab, substraction);
        add_function(context["*"], ab, multiplication);
        add_function(context["/"], ab, division);
        add_function(context["%"], ab, modulo);
        add_function(context["<"], ab, strictly_inf);
        add_function(context[">"], ab, strictly_sup);
        add_function(context["<="], ab, inf_equals);
        add_function(context[">="], ab, sup_equals);
        add_function(context["++"], a, increment);
        add_function(context["--"], a, decrement);
        add_function(context[":+="], ab, add);
        add_function(context[":-="], ab, remove);
        add_function(context[":*="], ab, mutiply);
        add_function(context[":/="], ab, divide);

        add_function(context["forall"], for_args, forall);
        add_function(context["exists"], for_args, exists);

        add_function(context["random"], std::make_shared<Parser::Tuple>(), random_0);
        add_function(context["random"], std::make_shared<Parser::Symbol>("b"), random_1);
        add_function(context["random"], ab, random_2);

        add_function(context["cos"], a, function1<std::cos>);
        add_function(context["sin"], a, function1<std::sin>);
        add_function(context["tan"], a, function1<std::tan>);
        add_function(context["acos"], a, function1<std::acos>);
        add_function(context["asin"], a, function1<std::asin>);
        add_function(context["atan"], a, function1<std::atan>);
        add_function(context["atan2"], ab, function2<std::atan2>);
        add_function(context["cosh"], a, function1<std::cosh>);
        add_function(context["sinh"], a, function1<std::sinh>);
        add_function(context["tanh"], a, function1<std::tanh>);
        add_function(context["acosh"], a, function1<std::acosh>);
        add_function(context["asinh"], a, function1<std::asinh>);
        add_function(context["atanh"], a, function1<std::atanh>);
        add_function(context["exp"], a, function1<std::exp>);
        add_function(context["log"], a, function1<std::log>);
        add_function(context["log10"], a, function1<std::log10>);
        add_function(context["pow"], ab, function2<std::pow>);
        Interpreter::set(context, context["**"], context["pow"]);
        Interpreter::set(context, context["^"], context["pow"]);
        add_function(context["**"], ab, function2<std::pow>);
        add_function(context["sqrt"], a, function1<std::sqrt>);
        add_function(context["cbrt"], a, function1<std::cbrt>);
        add_function(context["hypot"], ab, function2<std::hypot>);
        add_function(context["ceil"], a, function1<std::ceil>);
        add_function(context["floor"], a, function1<std::floor>);
        add_function(context["trunc"], a, function1<std::trunc>);
        add_function(context["round"], a, function1<std::round>);
        add_function(context["abs"], a, function1<std::abs>);
        add_function(context["isfinite"], a, function_bool<std::isfinite>);
        add_function(context["isinf"], a, function_bool<std::isinf>);
        add_function(context["isnan"], a, function_bool<std::isnan>);
        add_function(context["isnormal"], a, function_bool<std::isnormal>);

        Interpreter::set(context, context["epsilon"], Data(std::numeric_limits<OV_FLOAT>::epsilon()));
        Interpreter::set(context, context["infinity"], Data(std::numeric_limits<OV_FLOAT>::infinity()));
        Interpreter::set(context, context["NaN"], Data(std::numeric_limits<OV_FLOAT>::quiet_NaN()));
    }

}
