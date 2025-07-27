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

    Reference opposite(Data const& a) {
        if (auto const* a_int = get_if<OV_INT>(&a))
            return Data(-*a_int);
        else if (auto const* a_float = get_if<OV_FLOAT>(&a))
            return Data(-*a_float);
        throw FunctionArgumentsError();
    }

    Reference substraction(Data const& a, Data const& b) {
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

    Reference multiplication(Data const& a, Data const& b) {
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

    Reference division(Data const& a, Data const& b) {
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

    Reference modulo(OV_INT a, OV_INT b) {
        return Data(a % b);
    }

    Reference strictly_inf(Data const& a, Data const& b) {
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

    Reference strictly_sup(Data const& a, Data const& b) {
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

    Reference inf_equals(Data const& a, Data const& b) {
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

    Reference sup_equals(Data const& a, Data const& b) {
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
    Reference forall(FunctionContext& context, ObjectPtr const& array, Data const& function) {
        try {
            bool value = true;

            for (auto const& d : array->array) {
                if (!Interpreter::call_function(context.get_parent(), nullptr, function, d).to_data(context).get<bool>()) {
                    value = false;
                    break;
                }
            }

            return Data{ value };
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference exists(FunctionContext& context, ObjectPtr const& array, Data const& function) {
        try {
            bool value = false;

            for (auto const& d : array->array) {
                if (Interpreter::call_function(context.get_parent(), nullptr, function, d).to_data(context).get<bool>()) {
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

    Reference random_0() {
        std::uniform_real_distribution<OV_FLOAT> dis(0., 1.);
        return Data(dis(randgen));
    }

    Reference random_1(Data const& b) {
        if (auto const* b_int = get_if<OV_INT>(&b)) {
            std::uniform_int_distribution<OV_INT> dis(0, *b_int);
            return Data(dis(randgen));
        } else if (auto const* b_float = get_if<OV_FLOAT>(&b)) {
            std::uniform_real_distribution<OV_FLOAT> dis(0, *b_float);
            return Data(dis(randgen));
        }
        throw FunctionArgumentsError();
    }

    Reference random_2(Data const& a, Data const& b) {
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
    Reference function1(Data const& a) {
        return Data(function(get_OV_FLOAT(a)));
    }

    template<OV_FLOAT(*function)(OV_FLOAT, OV_FLOAT)>
    Reference function2(Data const& a, Data const& b) {
        return Data(function(get_OV_FLOAT(a), get_OV_FLOAT(b)));
    }

    template<bool (*function)(OV_FLOAT)>
    Reference function_bool(Data const& a) {
        return Data(function(get_OV_FLOAT(a)));
    }

    void init(GlobalContext& context) {
        add_function(context["!"], logical_not);
        add_function(context["&"], a_b, logical_and);
        add_function(context["|"], a_b, logical_or);
        add_function(context["+"], addition);
        add_function(context["-"], opposite);
        add_function(context["-"], substraction);
        add_function(context["*"], multiplication);
        add_function(context["/"], division);
        add_function(context["%"], modulo);
        add_function(context["<"], strictly_inf);
        add_function(context[">"], strictly_sup);
        add_function(context["<="], inf_equals);
        add_function(context[">="], sup_equals);
        add_function(context["++"], a, increment);
        add_function(context["--"], a, decrement);
        add_function(context[":+="], ab, add);
        add_function(context[":-="], ab, remove);
        add_function(context[":*="], ab, mutiply);
        add_function(context[":/="], ab, divide);

        add_function(context["forall"], forall);
        add_function(context["exists"], exists);

        add_function(context["random"], random_0);
        add_function(context["random"], random_1);
        add_function(context["random"], random_2);

        add_function(context["cos"], function1<std::cos>);
        add_function(context["sin"], function1<std::sin>);
        add_function(context["tan"], function1<std::tan>);
        add_function(context["acos"], function1<std::acos>);
        add_function(context["asin"], function1<std::asin>);
        add_function(context["atan"], function1<std::atan>);
        add_function(context["atan2"], function2<std::atan2>);
        add_function(context["cosh"], function1<std::cosh>);
        add_function(context["sinh"], function1<std::sinh>);
        add_function(context["tanh"], function1<std::tanh>);
        add_function(context["acosh"], function1<std::acosh>);
        add_function(context["asinh"], function1<std::asinh>);
        add_function(context["atanh"], function1<std::atanh>);
        add_function(context["exp"], function1<std::exp>);
        add_function(context["log"], function1<std::log>);
        add_function(context["log10"], function1<std::log10>);
        add_function(context["pow"], function2<std::pow>);
        Interpreter::set(context, context["**"], context["pow"]);
        Interpreter::set(context, context["^"], context["pow"]);
        add_function(context["**"], function2<std::pow>);
        add_function(context["sqrt"], function1<std::sqrt>);
        add_function(context["cbrt"], function1<std::cbrt>);
        add_function(context["hypot"], function2<std::hypot>);
        add_function(context["ceil"], function1<std::ceil>);
        add_function(context["floor"], function1<std::floor>);
        add_function(context["trunc"], function1<std::trunc>);
        add_function(context["round"], function1<std::round>);
        add_function(context["abs"], function1<std::abs>);
        add_function(context["isfinite"], function_bool<std::isfinite>);
        add_function(context["isinf"], function_bool<std::isinf>);
        add_function(context["isnan"], function_bool<std::isnan>);
        add_function(context["isnormal"], function_bool<std::isnormal>);

        Interpreter::set(context, context["epsilon"], Data(std::numeric_limits<OV_FLOAT>::epsilon()));
        Interpreter::set(context, context["infinity"], Data(std::numeric_limits<OV_FLOAT>::infinity()));
        Interpreter::set(context, context["NaN"], Data(std::numeric_limits<OV_FLOAT>::quiet_NaN()));
    }

}
