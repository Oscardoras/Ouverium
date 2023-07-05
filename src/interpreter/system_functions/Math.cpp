#include <algorithm>
#include <iostream>
#include <fstream>

#include "Math.hpp"


namespace Interpreter {

    namespace Math {

        auto a = std::make_shared<Parser::Symbol>("a");
        auto ab = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::Symbol>("b")
        }));
        auto a_b = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("b"),
                std::make_shared<Parser::Tuple>()
            )
        }));

        Reference logical_not(FunctionContext & context) {
            try {
                return Reference(Data(!static_cast<Data &>(context["a"]).get<bool>(context)));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        Reference logical_and(FunctionContext & context) {
            try {
                auto a = static_cast<Data &>(context["a"]).get<bool>(context);
                auto b = static_cast<Data &>(context["b"]).get<Object*>(context);

                if (a)
                    return Reference(Data(Interpreter::call_function(context.get_parent(), nullptr, b->functions, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>(context)));
                else
                    return Reference(Data(false));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        Reference logical_or(FunctionContext & context) {
            try {
                auto a = static_cast<Data &>(context["a"]).get<bool>(context);
                auto b = static_cast<Data &>(context["b"]).get<Object*>(context);

                if (!a)
                    return Interpreter::call_function(context.get_parent(), nullptr, b->functions, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>(context);
                else
                    return Reference(Data(true));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        Reference addition(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int + *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int + *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float + *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float + *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference opposite(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);

            if (auto a_int = std::get_if<long>(&a))
                return Reference(Data(- *a_int));
            else if (auto a_float = std::get_if<double>(&a))
                return Reference(Data(- *a_float));
            throw FunctionArgumentsError();
        }

        Reference substraction(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int - *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int - *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float - *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float - *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference multiplication(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int * *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int * *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float * *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float * *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference division(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int / *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int / *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float / *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float / *b_float));
            }
            throw FunctionArgumentsError();
        }

        Reference modulo(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a))
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int % *b_int));
            throw Interpreter::FunctionArgumentsError();
        }

        Reference strictly_inf(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int < *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int < *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float < *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float < *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char < *b_char));
            throw FunctionArgumentsError();
        }

        Reference strictly_sup(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int > *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int > *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float > *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float > *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char > *b_char));
            throw FunctionArgumentsError();
        }

        Reference inf_equals(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int <= *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int <= *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float <= *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float <= *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char <= *b_char));
            throw FunctionArgumentsError();
        }

        Reference sup_equals(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int >= *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_int >= *b_float));
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_float >= *b_int));
                else if (auto b_float = std::get_if<double>(&b))
                    return Reference(Data(*a_float >= *b_float));
            }
            if (auto a_char = std::get_if<char>(&a))
                if (auto b_char = std::get_if<char>(&b))
                    return Reference(Data(*a_char >= *b_char));
            throw FunctionArgumentsError();
        }

        Reference increment(FunctionContext & context) {
            auto & a = static_cast<Data &>(context["a"]);

            try {
                a = a.get<long>(context)+1;
                return SymbolReference(a);
            } catch (Data::BadAccess const& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference decrement(FunctionContext & context) {
            auto & a = static_cast<Data &>(context["a"]);

            try {
                a = a.get<long>(context)-1;
                return SymbolReference(a);
            } catch (Data::BadAccess const& e) {
                throw FunctionArgumentsError();
            }
        }

        Reference add(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int + *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int + *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float + *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float + *b_float);
            }
            throw FunctionArgumentsError();
        }

        Reference remove(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int - *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int - *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float - *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float - *b_float);
            }
            throw FunctionArgumentsError();
        }

        Reference mutiply(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int * *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int * *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float * *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float * *b_float);
            }
            throw FunctionArgumentsError();
        }

        Reference divide(FunctionContext & context) {
            auto a = static_cast<Data &>(context["a"]).compute(context);
            auto b = static_cast<Data &>(context["b"]).compute(context);

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int / *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_int / *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float / *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return static_cast<Data &>(context["a"]) = Data(*a_float / *b_float);
            }
            throw FunctionArgumentsError();
        }

        void init(GlobalContext & context) {
            context.get_function("!").push_front(SystemFunction{a, logical_not});
            context.get_function("&").push_front(SystemFunction{a_b, logical_and});
            context.get_function("|").push_front(SystemFunction{a_b, logical_or});
            context.get_function("+").push_front(SystemFunction{ab, addition});
            context.get_function("-").push_front(SystemFunction{a, opposite});
            context.get_function("-").push_front(SystemFunction{ab, substraction});
            context.get_function("*").push_front(SystemFunction{ab, multiplication});
            context.get_function("/").push_front(SystemFunction{ab, division});
            context.get_function("%").push_front(SystemFunction{ab, modulo});
            context.get_function("<").push_front(SystemFunction{ab, strictly_inf});
            context.get_function(">").push_front(SystemFunction{ab, strictly_sup});
            context.get_function("<=").push_front(SystemFunction{ab, inf_equals});
            context.get_function(">=").push_front(SystemFunction{ab, sup_equals});
            context.get_function("++").push_front(SystemFunction{a, increment});
            context.get_function("--").push_front(SystemFunction{a, decrement});
            context.get_function(":+").push_front(SystemFunction{ab, add});
            context.get_function(":-").push_front(SystemFunction{ab, remove});
            context.get_function(":*").push_front(SystemFunction{ab, mutiply});
            context.get_function(":/").push_front(SystemFunction{ab, divide});
        }

    }

}
