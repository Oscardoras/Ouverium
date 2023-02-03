#include <algorithm>
#include <iostream>
#include <fstream>

#include "Math.hpp"


namespace Interpreter {

    namespace Math {

        auto a = std::make_shared<Symbol>("a");
        auto ab = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("a"),
            std::make_shared<Symbol>("b")
        });
        auto a_b = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("a"),
            std::make_shared<FunctionCall>(
                std::make_shared<Symbol>("b"),
                std::make_shared<Tuple>()
            )
        });

        Reference logical_not(FunctionContext & context) {
            try {
                return Reference(Data(!context["a"].get<bool>()));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        Reference logical_and(FunctionContext & context) {
            try {
                auto a = context["a"].get<bool>();
                auto b = context["b"].get<Object*>();

                if (a)
                    return Reference(Data(Interpreter::call_function(context.get_parent(), nullptr, b->functions, std::make_shared<Tuple>()).to_data(context).get<bool>()));
                else
                    return Reference(Data(false));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        Reference logical_or(FunctionContext & context) {
            try {
                auto a = context["a"].get<bool>();
                auto b = context["b"].get<Object*>();

                if (!a)
                    return Interpreter::call_function(context.get_parent(), nullptr, b->functions, std::make_shared<Tuple>()).to_data(context).get<bool>();
                else
                    return Reference(Data(true));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        Reference addition(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

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
            auto a = context["a"];

            if (auto a_int = std::get_if<long>(&a))
                return Reference(Data(- *a_int));
            else if (auto a_float = std::get_if<double>(&a))
                return Reference(Data(- *a_float));
            throw FunctionArgumentsError();
        }

        Reference substraction(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

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
            auto a = context["a"];
            auto b = context["b"];

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
            auto a = context["a"];
            auto b = context["b"];

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
            auto a = context["a"];
            auto b = context["b"];

            if (auto a_int = std::get_if<long>(&a))
                if (auto b_int = std::get_if<long>(&b))
                    return Reference(Data(*a_int % *b_int));
            throw Interpreter::FunctionArgumentsError();
        }

        Reference strictly_inf(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

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
            throw FunctionArgumentsError();
        }

        Reference strictly_sup(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

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
            throw FunctionArgumentsError();
        }

        Reference inf_equals(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

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
            throw FunctionArgumentsError();
        }

        Reference sup_equals(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

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
            throw FunctionArgumentsError();
        }

        Reference increment(FunctionContext & context) {
            auto & a = context["a"];

            if (auto a_int = std::get_if<long>(&a)) {
                a = *a_int+1;
                return SymbolReference(a);
            }
            throw FunctionArgumentsError();
        }

        Reference decrement(FunctionContext & context) {
            auto & a = context["a"];

            if (auto a_int = std::get_if<long>(&a)) {
                a = *a_int-1;
                return SymbolReference(a);
            }
            throw FunctionArgumentsError();
        }

        Reference add(FunctionContext & context) {
            auto & a = context["a"];
            auto & b = context["b"];

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_int + *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_int + *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_float + *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_float + *b_float);
            }
            throw FunctionArgumentsError();
        }

        Reference remove(FunctionContext & context) {
            auto & a = context["a"];
            auto & b = context["b"];

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_int - *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_int - *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_float - *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_float - *b_float);
            }
            throw FunctionArgumentsError();
        }

        Reference mutiply(FunctionContext & context) {
            auto & a = context["a"];
            auto & b = context["b"];

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_int * *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_int * *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_float * *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_float * *b_float);
            }
            throw FunctionArgumentsError();
        }

        Reference divide(FunctionContext & context) {
            auto & a = context["a"];
            auto & b = context["b"];

            if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_int / *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_int / *b_float);
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_int = std::get_if<long>(&b))
                    return a = Data(*a_float / *b_int);
                else if (auto b_float = std::get_if<double>(&b))
                    return a = Data(*a_float / *b_float);
            }
            throw FunctionArgumentsError();
        }

        void init(Context & context) {
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
