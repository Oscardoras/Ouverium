#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <limits>

#include "Math.hpp"


namespace Interpreter {

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
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference logical_and(FunctionContext& context) {
            try {
                auto a = context["a"].to_data(context).get<bool>();
                auto b = context["b"].to_data(context).get<Object*>();

                if (a)
                    return Reference(Data(Interpreter::call_function(context.get_parent(), context.expression, b->functions, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>()));
                else
                    return Reference(Data(false));
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference logical_or(FunctionContext& context) {
            try {
                auto a = context["a"].to_data(context).get<bool>();
                auto b = context["b"].to_data(context).get<Object*>();

                if (!a)
                    return Interpreter::call_function(context.get_parent(), context.expression, b->functions, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>();
                else
                    return Reference(Data(true));
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference addition(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int + *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int + *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float + *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float + *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference opposite(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a))
                return Reference(Data(-*a_int));
            else if (auto a_float = std::get_if<FLOAT>(&a))
                return Reference(Data(-*a_float));
            throw FunctionArgumentsError();
        }

        Reference substraction(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int - *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int - *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float - *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float - *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference multiplication(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int * *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int * *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float * *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float * *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference division(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int / *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int / *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float / *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float / *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference modulo(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a))
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int % *b_int));
            throw Interpreter::FunctionArgumentsError();
        }

        Reference strictly_inf(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int < *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int < *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float < *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float < *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char < *b_char));
            throw FunctionArgumentsError();
        }

        Reference strictly_sup(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int > *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int > *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float > *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float > *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char > *b_char));
            throw FunctionArgumentsError();
        }

        Reference inf_equals(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int <= *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int <= *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float <= *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float <= *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char <= *b_char));
            throw FunctionArgumentsError();
        }

        Reference sup_equals(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_int >= *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_int >= *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return Reference(Data(*a_float >= *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return Reference(Data(*a_float >= *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char >= *b_char));
            throw FunctionArgumentsError();
        }

        Reference increment(FunctionContext& context) {
            auto a = context["a"];

            try {
                return set(context, a, a.to_data(context).get<INT>() + 1);
            } catch (Data::BadAccess const& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference decrement(FunctionContext& context) {
            auto a = context["a"];

            try {
                return set(context, a, a.to_data(context).get<INT>() - 1);
            } catch (Data::BadAccess const& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference add(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_int + *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int + *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_float + *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float + *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference remove(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_int - *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int - *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_float - *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float - *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference mutiply(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_int * *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int * *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_float * *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_float * *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference divide(FunctionContext& context) {
            auto a = context["a"].to_data(context);
            auto b = context["b"].to_data(context);

            if (auto a_int = std::get_if<INT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_int / *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
                    return set(context, context["a"], Data(*a_int / *b_float));
            } else if (auto a_float = std::get_if<FLOAT>(&a)) {
                if (auto b_int = std::get_if<INT>(&b))
                    return set(context, context["a"], Data(*a_float / *b_int));
                else if (auto b_float = std::get_if<FLOAT>(&b))
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
                auto array = Interpreter::call_function(context.get_parent(), context.expression, context["array"].to_data(context).get<Object*>()->functions, std::make_shared<Parser::Tuple>());
                auto functions = context["function"].to_data(context).get<Object*>()->functions;

                bool value = true;

                if (auto tuple = std::get_if<TupleReference>(&array)) {
                    for (auto const& r : *tuple) {
                        if (!Interpreter::call_function(context.get_parent(), context.expression, functions, r).to_data(context).get<bool>()) {
                            value = false;
                            break;
                        }
                    }
                } else {
                    for (auto const& d : array.to_data(context).get<Object*>()->array) {
                        if (!Interpreter::call_function(context.get_parent(), context.expression, functions, d).to_data(context).get<bool>()) {
                            value = false;
                            break;
                        }
                    }
                }

                return Data{ value };
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference exists(FunctionContext& context) {
            try {
                auto array = Interpreter::call_function(context.get_parent(), context.expression, context["array"].to_data(context).get<Object*>()->functions, std::make_shared<Parser::Tuple>());
                auto functions = context["function"].to_data(context).get<Object*>()->functions;

                bool value = false;

                if (auto tuple = std::get_if<TupleReference>(&array)) {
                    for (auto const& r : *tuple) {
                        if (Interpreter::call_function(context.get_parent(), context.expression, functions, r).to_data(context).get<bool>()) {
                            value = true;
                            break;
                        }
                    }
                } else {
                    for (auto const& d : array.to_data(context).get<Object*>()->array) {
                        if (Interpreter::call_function(context.get_parent(), context.expression, functions, d).to_data(context).get<bool>()) {
                            value = true;
                            break;
                        }
                    }
                }

                return Data{ value };
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        FLOAT get_FLOAT(Data const& data) {
            if (auto data_int = std::get_if<INT>(&data))
                return static_cast<FLOAT>(*data_int);
            else if (auto data_float = std::get_if<FLOAT>(&data))
                return *data_float;
            else throw FunctionArgumentsError();
        }

        template<FLOAT(*function)(FLOAT)>
        Reference function1(FunctionContext& context) {
            return Data(function(get_FLOAT(context["a"].to_data(context))));
        }

        template<FLOAT(*function)(FLOAT, FLOAT)>
        Reference function2(FunctionContext& context) {
            return Data(function(get_FLOAT(context["a"].to_data(context)), get_FLOAT(context["b"].to_data(context))));
        }

        template<bool (*function)(FLOAT)>
        Reference function_bool(FunctionContext& context) {
            return Data(function(get_FLOAT(context["a"].to_data(context))));
        }

        void init(GlobalContext& context) {
            context.get_function("!").push_front(SystemFunction{ a, logical_not });
            context.get_function("&").push_front(SystemFunction{ a_b, logical_and });
            context.get_function("|").push_front(SystemFunction{ a_b, logical_or });
            context.get_function("+").push_front(SystemFunction{ ab, addition });
            context.get_function("-").push_front(SystemFunction{ a, opposite });
            context.get_function("-").push_front(SystemFunction{ ab, substraction });
            context.get_function("*").push_front(SystemFunction{ ab, multiplication });
            context.get_function("/").push_front(SystemFunction{ ab, division });
            context.get_function("%").push_front(SystemFunction{ ab, modulo });
            context.get_function("<").push_front(SystemFunction{ ab, strictly_inf });
            context.get_function(">").push_front(SystemFunction{ ab, strictly_sup });
            context.get_function("<=").push_front(SystemFunction{ ab, inf_equals });
            context.get_function(">=").push_front(SystemFunction{ ab, sup_equals });
            context.get_function("++").push_front(SystemFunction{ a, increment });
            context.get_function("--").push_front(SystemFunction{ a, decrement });
            context.get_function(":+").push_front(SystemFunction{ ab, add });
            context.get_function(":-").push_front(SystemFunction{ ab, remove });
            context.get_function(":*").push_front(SystemFunction{ ab, mutiply });
            context.get_function(":/").push_front(SystemFunction{ ab, divide });
            context.get_function("forall").push_front(SystemFunction{ for_args, forall });
            context.get_function("exists").push_front(SystemFunction{ for_args, exists });

            context.get_function("cos").push_front(SystemFunction{ a, function1<std::cos> });
            context.get_function("sin").push_front(SystemFunction{ a, function1<std::sin> });
            context.get_function("tan").push_front(SystemFunction{ a, function1<std::tan> });
            context.get_function("acos").push_front(SystemFunction{ a, function1<std::acos> });
            context.get_function("asin").push_front(SystemFunction{ a, function1<std::asin> });
            context.get_function("atan").push_front(SystemFunction{ a, function1<std::atan> });
            context.get_function("atan2").push_front(SystemFunction{ ab, function2<std::atan2> });
            context.get_function("cosh").push_front(SystemFunction{ a, function1<std::cosh> });
            context.get_function("sinh").push_front(SystemFunction{ a, function1<std::sinh> });
            context.get_function("tanh").push_front(SystemFunction{ a, function1<std::tanh> });
            context.get_function("acosh").push_front(SystemFunction{ a, function1<std::acosh> });
            context.get_function("asinh").push_front(SystemFunction{ a, function1<std::asinh> });
            context.get_function("atanh").push_front(SystemFunction{ a, function1<std::atanh> });
            context.get_function("exp").push_front(SystemFunction{ a, function1<std::exp> });
            context.get_function("log").push_front(SystemFunction{ a, function1<std::log> });
            context.get_function("log10").push_front(SystemFunction{ a, function1<std::log10> });
            context.get_function("pow").push_front(SystemFunction{ ab, function2<std::pow> });
            context.add_symbol("**", context["pow"]);
            context.add_symbol("^", context["pow"]);
            context.get_function("**").push_front(SystemFunction{ ab, function2<std::pow> });
            context.get_function("sqrt").push_front(SystemFunction{ a, function1<std::sqrt> });
            context.get_function("cbrt").push_front(SystemFunction{ a, function1<std::cbrt> });
            context.get_function("hypot").push_front(SystemFunction{ ab, function2<std::hypot> });
            context.get_function("ceil").push_front(SystemFunction{ a, function1<std::ceil> });
            context.get_function("floor").push_front(SystemFunction{ a, function1<std::floor> });
            context.get_function("trunc").push_front(SystemFunction{ a, function1<std::trunc> });
            context.get_function("round").push_front(SystemFunction{ a, function1<std::round> });
            context.get_function("abs").push_front(SystemFunction{ a, function1<std::abs> });
            context.get_function("isfinite").push_front(SystemFunction{ a, function_bool<std::isfinite> });
            context.get_function("isinf").push_front(SystemFunction{ a, function_bool<std::isinf> });
            context.get_function("isnan").push_front(SystemFunction{ a, function_bool<std::isnan> });
            context.get_function("isnormal").push_front(SystemFunction{ a, function_bool<std::isnormal> });

            context.add_symbol("epsilon", Data(std::numeric_limits<FLOAT>::epsilon()));
            context.add_symbol("infinity", Data(std::numeric_limits<FLOAT>::infinity()));
            context.add_symbol("NaN", Data(std::numeric_limits<FLOAT>::quiet_NaN()));
        }

    }

}
