#include "Array.hpp"


namespace Interpreter::SystemFunctions {

    namespace Array {

        auto length_args = std::make_shared<Parser::Symbol>("array");
        Reference length(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<Object*>();

                return Reference(Data((INT) array->array.size()));
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto get_capacity_args = std::make_shared<Parser::Symbol>("array");
        Reference get_capacity(FunctionContext& context) {
            auto array = context["array"].to_data(context).get<Object*>();

            try {
                return Data((INT) array->array.capacity());
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto set_capacity_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("array"),
                std::make_shared<Parser::Symbol>("capacity")
            }
        ));
        Reference set_capacity(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<Object*>();
                auto capacity = context["capacity"].to_data(context).get<INT>();

                array->array.reserve(capacity);
                return Data();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto get_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("array"),
                std::make_shared<Parser::Symbol>("i")
            }
        ));
        Reference get(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<Object*>();
                auto i = context["i"].to_data(context).get<INT>();

                if (i >= 0 && i < (INT) array->array.size())
                    return ArrayReference{ *array, (size_t) i };
                else throw FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto add_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("array"),
                std::make_shared<Parser::Symbol>("element")
            }
        ));
        Reference add(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<Object*>();
                auto element = context["element"].to_data(context);

                array->array.push_back(element);
                return ArrayReference{ *array, (size_t) array->array.size() - 1 };
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto remove_args = std::make_shared<Parser::Symbol>("array");
        Reference remove(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<Object*>();

                Data d = array->array.back();
                array->array.pop_back();
                return Data(d);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto foreach_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
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
                auto array = Interpreter::call_function(context.get_parent(), context.expression, context["array"], std::make_shared<Parser::Tuple>());

                if (auto tuple = std::get_if<TupleReference>(&array)) {
                    for (auto const& r : *tuple)
                        Interpreter::call_function(context.get_parent(), context.expression, context["function"], r);
                } else {
                    for (auto const& d : array.to_data(context).get<Object*>()->array)
                        Interpreter::call_function(context.get_parent(), context.expression, context["function"], d);
                }

                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        void init(GlobalContext& context) {
            auto array = context["Array"].to_data(context).get<Object*>();
            (*array)["length"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ length_args, length });
            (*array)["get_capacity"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ get_capacity_args, get_capacity });
            (*array)["set_capacity"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ set_capacity_args, set_capacity });
            (*array)["get"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ get_args, get });
            (*array)["add"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ add_args, add });
            (*array)["remove"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ remove_args, remove });

            context.get_function("foreach").push_front(SystemFunction{ foreach_args, foreach });
        }

    }

}
