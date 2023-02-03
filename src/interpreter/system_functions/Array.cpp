#include <algorithm>
#include <iostream>
#include <fstream>

#include "Array.hpp"


namespace Interpreter {

    namespace Array {

        auto lenght_args = std::make_shared<Symbol>("array");
        Reference lenght(FunctionContext & context) {
            try {
                auto array = context["array"].get<Object*>();

                return Reference(Data((long) array->array.size()));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto get_capacity_args = std::make_shared<Symbol>("array");
        Reference get_capacity(FunctionContext & context) {
            auto array = context["array"].get<Object*>();

            try {
                return Data((long) array->array.capacity());
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto set_capacity_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("array"),
            std::make_shared<Symbol>("capacity")
        });
        Reference set_capacity(FunctionContext & context) {
            try {
                auto array = context["array"].get<Object*>();
                auto capacity = context["capacity"].get<long>();

                array->array.reserve(capacity);
                return Data();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto get_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("array"),
            std::make_shared<Symbol>("i")
        });
        Reference get(FunctionContext & context) {
            try {
                auto array = context["array"].get<Object*>();
                auto i = context["i"].get<long>();

                if (i >= 0 && i < array->array.size())
                    return ArrayReference{*array, (size_t) i};
                else throw FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto add_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("array"),
            std::make_shared<Symbol>("element")
        });
        Reference add(FunctionContext & context) {
            try {
                auto array = context["array"].get<Object*>();
                auto element = context["element"];

                array->array.push_back(element);
                return ArrayReference{*array, (size_t) array->array.size()-1};
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto remove_args = std::make_shared<Symbol>("array");
        Reference remove(FunctionContext & context) {
            try {
                auto array = context["array"].get<Object*>();
                auto element = context["element"];

                Data d = array->array.back();
                array->array.pop_back();
                return Data(d);
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        void init(Context & context) {
            auto array = context["Array"].get<Object*>();
            array->get_property("lenght", context).get<Object*>()->functions.push_front(SystemFunction{lenght_args, lenght});
            array->get_property("get_capacity", context).get<Object*>()->functions.push_front(SystemFunction{get_capacity_args, get_capacity});
            array->get_property("set_capacity", context).get<Object*>()->functions.push_front(SystemFunction{set_capacity_args, set_capacity});
            array->get_property("get", context).get<Object*>()->functions.push_front(SystemFunction{get_args, get});
            array->get_property("add", context).get<Object*>()->functions.push_front(SystemFunction{add_args, add});
            array->get_property("remove", context).get<Object*>()->functions.push_front(SystemFunction{remove_args, remove});
        }

    }

}
