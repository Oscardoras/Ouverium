#include "Types.hpp"


namespace Interpreter::SystemFunctions {

    namespace Types {

        auto constructor_args = std::make_shared<Parser::Symbol>("a");

        Reference char_constructor(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (a.is<char>())
                return a;
            else
                throw FunctionArgumentsError();
        }

        Reference float_constructor(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (auto a_int = get_if<OV_INT>(&a)) {
                return Data((OV_FLOAT) *a_int);
            } else if (a.is<OV_FLOAT>()) {
                return a;
            }
            throw FunctionArgumentsError();
        }

        Reference int_constructor(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (a.is<OV_INT>()) {
                return a;
            } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
                return Data((OV_INT) *a_float);
            }
            throw FunctionArgumentsError();
        }

        Reference bool_constructor(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (a.is<bool>())
                return a;
            else
                throw FunctionArgumentsError();
        }

        Reference array_constructor(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (auto obj = get_if<Object*>(&a); obj && (*obj)->array.capacity() > 0)
                return a;
            else
                throw FunctionArgumentsError();
        }

        auto tuple_constructor_args = std::make_shared<Parser::FunctionCall>(
            std::make_shared<Parser::Symbol>("tuple"),
            std::make_shared<Parser::Tuple>()
        );
        Reference tuple_constructor(FunctionContext& context) {
            auto tuple = Interpreter::call_function(context.get_parent(), context.expression, context["tuple"], std::make_shared<Parser::Tuple>());

            if (std::holds_alternative<TupleReference>(tuple))
                return tuple;
            else {
                return TupleReference{ tuple };
            }
        }

        Reference function_constructor(FunctionContext& context) {
            auto a = context["a"].to_data(context);

            if (auto obj = get_if<Object*>(&a); obj && !(*obj)->functions.empty())
                return a;
            else
                throw FunctionArgumentsError();
        }

        bool check_type(Context& context, Data data, Data type) {
            if (type == context["Char"].to_data(context)) return data.is<char>();
            else if (type == context["Float"].to_data(context)) return data.is<OV_FLOAT>();
            else if (type == context["Int"].to_data(context)) return data.is<OV_INT>();
            else if (type == context["Bool"].to_data(context)) return data.is<bool>();
            else if (type == context["Array"].to_data(context)) {
                if (auto obj = get_if<Object*>(&data))
                    return (*obj)->array.capacity() > 0;
                else
                    return false;
            } else if (type == context["Function"].to_data(context)) {
                if (auto obj = get_if<Object*>(&data))
                    return !(*obj)->functions.empty();
                else
                    return false;
            } else throw FunctionArgumentsError();
        }

        auto is_type_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("data"),
                std::make_shared<Parser::Symbol>("type")
            }
        ));
        Reference is_type(FunctionContext& context) {
            auto data = context["data"].to_data(context);
            auto type = context["type"].to_data(context);

            return Data(check_type(context, data, type));
        }

        void init(GlobalContext& context) {
            insert(context, "Char")->functions.push_front(SystemFunction{ constructor_args, char_constructor });
            insert(context, "Float")->functions.push_front(SystemFunction{ constructor_args, float_constructor });
            insert(context, "Int")->functions.push_front(SystemFunction{ constructor_args, int_constructor });
            insert(context, "Bool")->functions.push_front(SystemFunction{ constructor_args, bool_constructor });
            insert(context, "Array")->functions.push_front(SystemFunction{ constructor_args, array_constructor });
            insert(context, "Tuple")->functions.push_front(SystemFunction{ tuple_constructor_args, tuple_constructor });
            insert(context, "Function")->functions.push_front(SystemFunction{ constructor_args, function_constructor });

            Function f = SystemFunction{ is_type_args, is_type };
            f.extern_symbols.emplace("Char", context["Char"]);
            f.extern_symbols.emplace("Float", context["Float"]);
            f.extern_symbols.emplace("Int", context["Int"]);
            f.extern_symbols.emplace("Bool", context["Bool"]);
            f.extern_symbols.emplace("Array", context["Array"]);
            f.extern_symbols.emplace("Function", context["Function"]);
            insert(context, "~")->functions.push_front(f);
        }

    }

}
