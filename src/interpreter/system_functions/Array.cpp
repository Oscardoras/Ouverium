#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace Array {

        auto get_capacity_args = std::make_shared<Parser::Symbol>("array");
        Reference get_capacity(FunctionContext& context) {
            auto array = context["array"].to_data(context).get<ObjectPtr>();

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
                auto array = context["array"].to_data(context).get<ObjectPtr>();
                auto capacity = context["capacity"].to_data(context).get<OV_INT>();

                array->array.reserve(capacity);
                return Data();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto get_size_args = std::make_shared<Parser::Symbol>("array");
        Reference get_size(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<ObjectPtr>();

                return Data((OV_INT) array->array.size());
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto set_size_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("array"),
                std::make_shared<Parser::Symbol>("size")
            }
        ));
        Reference set_size(FunctionContext& context) {
            try {
                auto array = context["array"].to_data(context).get<ObjectPtr>();
                auto size = context["size"].to_data(context).get<OV_INT>();

                array->array.resize(size);
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
                auto array = context["array"].to_data(context).get<ObjectPtr>();
                auto i = context["i"].to_data(context).get<OV_INT>();

                if (i >= 0 && i < (OV_INT) array->array.size())
                    return ArrayReference{ Data(array), static_cast<size_t>(i) };
                else throw FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto copy_data_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("from_array"),
                std::make_shared<Parser::Symbol>("from_i"),
                std::make_shared<Parser::Symbol>("to_array"),
                std::make_shared<Parser::Symbol>("to_i"),
                std::make_shared<Parser::Symbol>("n")
            }
        ));
        Reference copy_data(FunctionContext& context) {
            try {
                auto& from_array = context["from_array"].to_data(context).get<ObjectPtr>()->array;
                auto from_i = context["from_i"].to_data(context).get<OV_INT>();
                auto& to_array = context["to_array"].to_data(context).get<ObjectPtr>()->array;
                auto to_i = context["to_i"].to_data(context).get<OV_INT>();
                auto n = context["n"].to_data(context).get<OV_INT>();

                if (n < 0)
                    throw FunctionArgumentsError();
                else if (n == 0)
                    return Data{};
                if (from_i < 0 || from_i + static_cast<size_t>(n) > from_array.size())
                    throw FunctionArgumentsError();
                if (to_i < 0 || to_i + static_cast<size_t>(n) > to_array.size())
                    throw FunctionArgumentsError();

                if (from_i < to_i) {
                    for (OV_INT i = n - 1; i >= 0; --i)
                        to_array[to_i + i] = from_array[from_i + i];
                } else {
                    for (OV_INT i = 0; i < n; ++i)
                        to_array[to_i + i] = from_array[from_i + i];
                }

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
                auto array = Interpreter::call_function(context.get_parent(), nullptr, context["array"], std::make_shared<Parser::Tuple>());

                if (auto tuple = std::get_if<TupleReference>(&array)) {
                    for (auto const& r : *tuple)
                        Interpreter::call_function(context.get_parent(), nullptr, context["function"], r);
                } else {
                    auto obj = array.to_data(context).get<ObjectPtr>();
                    size_t size = obj->array.size();
                    for (size_t i = 0; i < size; ++i)
                        Interpreter::call_function(context.get_parent(), nullptr, context["function"], ArrayReference{ Data(obj) , i });
                }

                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto function_extract_args = std::make_shared<Parser::Symbol>("function");
        Reference function_extract(FunctionContext& context) {
            try {
                auto const& functions = context["function"].to_data(context).get<ObjectPtr>()->functions;

                auto object = GC::new_object();
                object->array.reserve(std::min(static_cast<size_t>(1), functions.size()));
                for (auto const& f : functions) {
                    auto obj = GC::new_object();
                    obj->functions.push_front(f);
                    object->array.push_back(Data(obj));
                }

                return Data(object);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto function_clear_args = std::make_shared<Parser::Symbol>("function");
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
            get_object(array->properties["get_capacity"])->functions.push_front(SystemFunction{ get_capacity_args, get_capacity });
            get_object(array->properties["set_capacity"])->functions.push_front(SystemFunction{ set_capacity_args, set_capacity });
            get_object(array->properties["get_size"])->functions.push_front(SystemFunction{ get_size_args, get_size });
            get_object(array->properties["set_size"])->functions.push_front(SystemFunction{ set_size_args, set_size });
            get_object(array->properties["get"])->functions.push_front(SystemFunction{ get_args, get });
            get_object(array->properties["copy_data"])->functions.push_front(SystemFunction{ copy_data_args, copy_data });

            get_object(context["foreach"])->functions.push_front(SystemFunction{ foreach_args, foreach });

            auto function = get_object(context["Function"]);
            get_object(function->properties["extract"])->functions.push_front(SystemFunction{ function_extract_args, function_extract });
            get_object(function->properties["clear"])->functions.push_front(SystemFunction{ function_clear_args, function_clear });
        }

    }

}
