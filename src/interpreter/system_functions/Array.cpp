#include "Array.hpp"


namespace Interpreter::SystemFunctions {

    namespace Array {

        auto length_args = std::make_shared<Parser::Symbol>("array");
        Reference length(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<Object*>();

                return Reference(Data((OV_INT) array->array.size()));
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto get_capacity_args = std::make_shared<Parser::Symbol>("array");
        Reference get_capacity(FunctionContext& context) {
            auto array = context["array"].to_data(context).get<Object*>();

            try {
                return Data((OV_INT) array->array.capacity());
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
                auto capacity = context["capacity"].to_data(context).get<OV_INT>();

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
                auto i = context["i"].to_data(context).get<OV_INT>();

                if (i >= 0 && i < (OV_INT) array->array.size())
                    return ArrayReference{ Data(array), static_cast<size_t>(i) };
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
                return ArrayReference{ Data(array), array->array.size() - 1 };
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

        auto shift_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("array"),
                std::make_shared<Parser::Symbol>("n")
            }
        ));
        Reference shift(FunctionContext& context) {
            try {
                auto& array = context["array"].to_data(context).get<Object*>()->array;
                auto n = context["n"].to_data(context).get<OV_INT>();

                if (n > 0) {
                    for (OV_INT i = array.size() - 1; i >= n; --i) {
                        array[i] = array[i - n];
                    }
                } else {
                    for (OV_INT i = 0; i < static_cast<OV_INT>(array.size()) + n; ++i) {
                        array[i] = array[i - n];
                    }
                }

                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto concat_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("from"),
                std::make_shared<Parser::Symbol>("to")
            }
        ));
        Reference concat(FunctionContext& context) {
            try {
                auto& from = context["from"].to_data(context).get<Object*>()->array;
                auto& to = context["to"].to_data(context).get<Object*>()->array;

                from.reserve(from.size() + to.size());
                for (auto const& object : to)
                    from.push_back(object);

                return Data{};
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
                    auto obj = array.to_data(context).get<Object*>();
                    size_t size = obj->array.size();
                    for (size_t i = 0; i < size; ++i)
                        Interpreter::call_function(context.get_parent(), context.expression, context["function"], ArrayReference{ Data(obj) , i });
                }

                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        void init(GlobalContext& context) {
            auto array = insert(context, "Array");
            insert(context, array, "length")->functions.push_front(SystemFunction{ length_args, length });
            insert(context, array, "get_capacity")->functions.push_front(SystemFunction{ get_capacity_args, get_capacity });
            insert(context, array, "set_capacity")->functions.push_front(SystemFunction{ set_capacity_args, set_capacity });
            insert(context, array, "get")->functions.push_front(SystemFunction{ get_args, get });
            insert(context, array, "add")->functions.push_front(SystemFunction{ add_args, add });
            insert(context, array, "remove")->functions.push_front(SystemFunction{ remove_args, remove });
            insert(context, array, "shift")->functions.push_front(SystemFunction{ shift_args, shift });
            insert(context, array, "concat")->functions.push_front(SystemFunction{ concat_args, concat });

            insert(context, "foreach")->functions.push_front(SystemFunction{ foreach_args, foreach });
        }

    }

}
