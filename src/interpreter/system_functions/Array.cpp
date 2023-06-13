#include <algorithm>
#include <iostream>
#include <fstream>

#include "Array.hpp"


namespace Interpreter {

    namespace Array {

        auto lenght_args = std::make_shared<Parser::Symbol>("array");
        Reference lenght(FunctionContext & context) {
            try {
                auto array = static_cast<Data &>(context["array"]).get<Object*>(context);

                return Reference(Data((long) array->array.size()));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto get_capacity_args = std::make_shared<Parser::Symbol>("array");
        Reference get_capacity(FunctionContext & context) {
            auto array = static_cast<Data &>(context["array"]).get<Object*>(context);

            try {
                return Data((long) array->array.capacity());
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto set_capacity_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("array"),
            std::make_shared<Parser::Symbol>("capacity")
        }));
        Reference set_capacity(FunctionContext & context) {
            try {
                auto array = static_cast<Data &>(context["array"]).get<Object*>(context);
                auto capacity = static_cast<Data &>(context["capacity"]).get<long>(context);

                array->array.reserve(capacity);
                return Data();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto get_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("array"),
            std::make_shared<Parser::Symbol>("i")
        }));
        Reference get(FunctionContext & context) {
            try {
                auto array = static_cast<Data &>(context["array"]).get<Object*>(context);
                auto i = static_cast<Data &>(context["i"]).get<long>(context);

                if (i >= 0 && i < (long) array->array.size())
                    return ArrayReference{*array, (size_t) i};
                else throw FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto add_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("array"),
            std::make_shared<Parser::Symbol>("element")
        }));
        Reference add(FunctionContext & context) {
            try {
                auto array = static_cast<Data &>(context["array"]).get<Object*>(context);
                auto element = context["element"];

                array->array.push_back(element);
                return ArrayReference{*array, (size_t) array->array.size()-1};
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto remove_args = std::make_shared<Parser::Symbol>("array");
        Reference remove(FunctionContext & context) {
            try {
                auto array = static_cast<Data &>(context["array"]).get<Object*>(context);

                Data d = array->array.back();
                array->array.pop_back();
                return Data(d);
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        void init(Context & context) {
            auto array = static_cast<Data &>(context["Array"]).get<Object*>(context);
            array->get_property("lenght", context).get<Object*>(context)->functions.push_front(SystemFunction{lenght_args, lenght});
            array->get_property("get_capacity", context).get<Object*>(context)->functions.push_front(SystemFunction{get_capacity_args, get_capacity});
            array->get_property("set_capacity", context).get<Object*>(context)->functions.push_front(SystemFunction{set_capacity_args, set_capacity});
            array->get_property("get", context).get<Object*>(context)->functions.push_front(SystemFunction{get_args, get});
            array->get_property("add", context).get<Object*>(context)->functions.push_front(SystemFunction{add_args, add});
            array->get_property("remove", context).get<Object*>(context)->functions.push_front(SystemFunction{remove_args, remove});
        }

    }

}
