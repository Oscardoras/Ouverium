#include <cmath>
#include <cstdlib>
#include <limits>

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace Math {

        auto a = std::make_shared<Parser::Symbol>("a");
        auto ab = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("a"),
                std::make_shared<Parser::Symbol>("b")
            }
        ));
        auto a_b = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("a"),
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("b"),
                    std::make_shared<Parser::Tuple>()
                )
            }
        ));

        Reference logical_not(FunctionContext& context) {
            try {
                return Reference(Data(!context["a"].to_data(context).get<bool>()));
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
                    return Reference(Data(false));
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
                    return Reference(Data(true));
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference addition(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int + *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int + *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float + *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float + *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference opposite(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a))
                return Reference(Data(-*a_int));
            else if (auto a_float = get_if<OV_FLOAT>(&a))
                return Reference(Data(-*a_float));
            throw FunctionArgumentsError();
        }

        Reference substraction(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int - *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int - *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float - *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float - *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference multiplication(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int * *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int * *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float * *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float * *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference division(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int / *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int / *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float / *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float / *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference modulo(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a))
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int % *b_int));
            throw Interpreter::FunctionArgumentsError();
        }

        Reference strictly_inf(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int < *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int < *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float < *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float < *b_float));
            }
            if (auto a_char = get_if<char>(&a))
                if (auto b_char = get_if<char>(&b))
                    return Reference(Data(*a_char < *b_char));
            throw FunctionArgumentsError();
        }

        Reference strictly_sup(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int > *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int > *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float > *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float > *b_float));
            }
            if (auto a_char = get_if<char>(&a))
                if (auto b_char = get_if<char>(&b))
                    return Reference(Data(*a_char > *b_char));
            throw FunctionArgumentsError();
        }

        Reference inf_equals(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int <= *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int <= *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float <= *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float <= *b_float));
            }
            if (auto a_char = get_if<char>(&a))
                if (auto b_char = get_if<char>(&b))
                    return Reference(Data(*a_char <= *b_char));
            throw FunctionArgumentsError();
        }

        Reference sup_equals(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_int >= *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_int >= *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Reference(Data(*a_float >= *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Reference(Data(*a_float >= *b_float));
            }
            if (auto a_char = get_if<char>(&a))
                if (auto b_char = get_if<char>(&b))
                    return Reference(Data(*a_char >= *b_char));
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

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_int + *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int + *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_float + *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float + *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference remove(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_int - *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int - *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_float - *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float - *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference mutiply(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_int * *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int * *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_float * *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float * *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference divide(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_int / *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int / *b_float));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return set(context, context["a"], Data(*a_float / *b_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float / *b_float));
            }
            throw FunctionArgumentsError();
        }

        auto for_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::FunctionCall>(
                    std::make_shared<Parser::Symbol>("array"),
                    std::make_shared<Parser::Tuple>()
                ),
                std::make_shared<Parser::Symbol>("function")
            }
        ));
        Reference forall(FunctionContext& context) {
            try {
                auto array = Interpreter::call_function(context.get_parent(), nullptr, context["array"], std::make_shared<Parser::Tuple>());
                auto functions = context["function"];

                bool value = true;

                if (auto tuple = std::get_if<TupleReference>(&array)) {
                    for (auto const& r : *tuple) {
                        if (!Interpreter::call_function(context.get_parent(), nullptr, functions, r).to_data(context).get<bool>()) {
                            value = false;
                            break;
                        }
                    }
                } else {
                    for (auto const& d : array.to_data(context).get<ObjectPtr>()->array) {
                        if (!Interpreter::call_function(context.get_parent(), nullptr, functions, d).to_data(context).get<bool>()) {
                            value = false;
                            break;
                        }
                    }
                }

                return Data{ value };
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference exists(FunctionContext& context) {
            try {
                auto array = Interpreter::call_function(context.get_parent(), nullptr, context["array"], std::make_shared<Parser::Tuple>());
                auto functions = context["function"];

                bool value = false;

                if (auto tuple = std::get_if<TupleReference>(&array)) {
                    for (auto const& r : *tuple) {
                        if (Interpreter::call_function(context.get_parent(), nullptr, functions, r).to_data(context).get<bool>()) {
                            value = true;
                            break;
                        }
                    }
                } else {
                    for (auto const& d : array.to_data(context).get<ObjectPtr>()->array) {
                        if (Interpreter::call_function(context.get_parent(), nullptr, functions, d).to_data(context).get<bool>()) {
                            value = true;
                            break;
                        }
                    }
                }

                return Data{ value };
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference random_0(FunctionContext&) {
            return Data(static_cast<OV_INT>(std::rand() / (OV_FLOAT) (RAND_MAX + 1u)));
        }

        Reference random_1(FunctionContext& context) {
            auto b = context["b"].to_data(context);

            if (auto b_int = get_if<OV_INT>(&b))
                return Data(static_cast<OV_INT>(std::rand() / ((RAND_MAX + 1u) / *b_int)));
            else if (auto b_float = get_if<OV_FLOAT>(&b))
                return Data(static_cast<OV_INT>(std::rand() / ((RAND_MAX + 1u) / *b_float)));
            throw FunctionArgumentsError();
        }

        Reference random_2(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Data(static_cast<OV_INT>(std::rand() / ((RAND_MAX + 1u) / (*b_int - *a_int)) + *a_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Data(static_cast<OV_INT>(std::rand() / ((RAND_MAX + 1u) / (*b_float - *a_int)) + *a_int));
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                if (auto b_int = get_if<OV_INT>(&b))
                    return Data(static_cast<OV_INT>(std::rand() / ((RAND_MAX + 1u) / (*b_int - *a_int)) + *a_int));
                else if (auto b_float = get_if<OV_FLOAT>(&b))
                    return Data(static_cast<OV_INT>(std::rand() / ((RAND_MAX + 1u) / (*b_float - *a_float)) + *a_float));
            }
            throw FunctionArgumentsError();
        }

        OV_FLOAT get_OV_FLOAT(Data const& data) {
            if (auto data_int = get_if<OV_INT>(&data))
                return static_cast<OV_FLOAT>(*data_int);
            else if (auto data_float = get_if<OV_FLOAT>(&data))
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
            get_object(context["!"])->functions.push_front(SystemFunction{ a, logical_not });
            get_object(context["&"])->functions.push_front(SystemFunction{ a_b, logical_and });
            get_object(context["|"])->functions.push_front(SystemFunction{ a_b, logical_or });
            get_object(context["+"])->functions.push_front(SystemFunction{ ab, addition });
            get_object(context["-"])->functions.push_front(SystemFunction{ a, opposite });
            get_object(context["-"])->functions.push_front(SystemFunction{ ab, substraction });
            get_object(context["*"])->functions.push_front(SystemFunction{ ab, multiplication });
            get_object(context["/"])->functions.push_front(SystemFunction{ ab, division });
            get_object(context["%"])->functions.push_front(SystemFunction{ ab, modulo });
            get_object(context["<"])->functions.push_front(SystemFunction{ ab, strictly_inf });
            get_object(context[">"])->functions.push_front(SystemFunction{ ab, strictly_sup });
            get_object(context["<="])->functions.push_front(SystemFunction{ ab, inf_equals });
            get_object(context[">="])->functions.push_front(SystemFunction{ ab, sup_equals });
            get_object(context["++"])->functions.push_front(SystemFunction{ a, increment });
            get_object(context["--"])->functions.push_front(SystemFunction{ a, decrement });
            get_object(context[":+="])->functions.push_front(SystemFunction{ ab, add });
            get_object(context[":-="])->functions.push_front(SystemFunction{ ab, remove });
            get_object(context[":*="])->functions.push_front(SystemFunction{ ab, mutiply });
            get_object(context[":/="])->functions.push_front(SystemFunction{ ab, divide });

            get_object(context["forall"])->functions.push_front(SystemFunction{ for_args, forall });
            get_object(context["exists"])->functions.push_front(SystemFunction{ for_args, exists });

            get_object(context["random"])->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), random_0 });
            get_object(context["random"])->functions.push_front(SystemFunction{ std::make_shared<Parser::Symbol>("b"), random_1 });
            get_object(context["random"])->functions.push_front(SystemFunction{ ab, random_2 });

            get_object(context["cos"])->functions.push_front(SystemFunction{ a, function1<std::cos> });
            get_object(context["sin"])->functions.push_front(SystemFunction{ a, function1<std::sin> });
            get_object(context["tan"])->functions.push_front(SystemFunction{ a, function1<std::tan> });
            get_object(context["acos"])->functions.push_front(SystemFunction{ a, function1<std::acos> });
            get_object(context["asin"])->functions.push_front(SystemFunction{ a, function1<std::asin> });
            get_object(context["atan"])->functions.push_front(SystemFunction{ a, function1<std::atan> });
            get_object(context["atan2"])->functions.push_front(SystemFunction{ ab, function2<std::atan2> });
            get_object(context["cosh"])->functions.push_front(SystemFunction{ a, function1<std::cosh> });
            get_object(context["sinh"])->functions.push_front(SystemFunction{ a, function1<std::sinh> });
            get_object(context["tanh"])->functions.push_front(SystemFunction{ a, function1<std::tanh> });
            get_object(context["acosh"])->functions.push_front(SystemFunction{ a, function1<std::acosh> });
            get_object(context["asinh"])->functions.push_front(SystemFunction{ a, function1<std::asinh> });
            get_object(context["atanh"])->functions.push_front(SystemFunction{ a, function1<std::atanh> });
            get_object(context["exp"])->functions.push_front(SystemFunction{ a, function1<std::exp> });
            get_object(context["log"])->functions.push_front(SystemFunction{ a, function1<std::log> });
            get_object(context["log10"])->functions.push_front(SystemFunction{ a, function1<std::log10> });
            get_object(context["pow"])->functions.push_front(SystemFunction{ ab, function2<std::pow> });
            Interpreter::set(context, context["**"], context["pow"]);
            Interpreter::set(context, context["^"], context["pow"]);
            get_object(context["**"])->functions.push_front(SystemFunction{ ab, function2<std::pow> });
            get_object(context["sqrt"])->functions.push_front(SystemFunction{ a, function1<std::sqrt> });
            get_object(context["cbrt"])->functions.push_front(SystemFunction{ a, function1<std::cbrt> });
            get_object(context["hypot"])->functions.push_front(SystemFunction{ ab, function2<std::hypot> });
            get_object(context["ceil"])->functions.push_front(SystemFunction{ a, function1<std::ceil> });
            get_object(context["floor"])->functions.push_front(SystemFunction{ a, function1<std::floor> });
            get_object(context["trunc"])->functions.push_front(SystemFunction{ a, function1<std::trunc> });
            get_object(context["round"])->functions.push_front(SystemFunction{ a, function1<std::round> });
            get_object(context["abs"])->functions.push_front(SystemFunction{ a, function1<std::abs> });
            get_object(context["isfinite"])->functions.push_front(SystemFunction{ a, function_bool<std::isfinite> });
            get_object(context["isinf"])->functions.push_front(SystemFunction{ a, function_bool<std::isinf> });
            get_object(context["isnan"])->functions.push_front(SystemFunction{ a, function_bool<std::isnan> });
            get_object(context["isnormal"])->functions.push_front(SystemFunction{ a, function_bool<std::isnormal> });

            Interpreter::set(context, context["epsilon"], Data(std::numeric_limits<OV_FLOAT>::epsilon()));
            Interpreter::set(context, context["infinity"], Data(std::numeric_limits<OV_FLOAT>::infinity()));
            Interpreter::set(context, context["NaN"], Data(std::numeric_limits<OV_FLOAT>::quiet_NaN()));
        }

    }

}
