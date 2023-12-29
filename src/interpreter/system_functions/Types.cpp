#include "Array.hpp"
#include "Types.hpp"


namespace Interpreter {

    namespace Types {

        bool check_type(Context& context, Data data, Data type) {
            if (type == context["Char"].to_data(context)) return std::holds_alternative<char>(data);
            else if (type == context["Float"].to_data(context)) return std::holds_alternative<double>(data);
            else if (type == context["Int"].to_data(context)) return std::holds_alternative<long>(data);
            else if (type == context["Bool"].to_data(context)) return std::holds_alternative<bool>(data);
            else if (type == context["Array"].to_data(context)) return std::holds_alternative<Object*>(data);
            else throw FunctionArgumentsError();
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
            context["Char"].to_data(context);
            context["Float"].to_data(context);
            context["Int"].to_data(context);
            context["Bool"].to_data(context);
            context["Array"].to_data(context);

            Function f = SystemFunction{ is_type_args, is_type };
            f.extern_symbols.emplace("Char", context["Char"]);
            f.extern_symbols.emplace("Float", context["Float"]);
            f.extern_symbols.emplace("Int", context["Int"]);
            f.extern_symbols.emplace("Bool", context["Bool"]);
            f.extern_symbols.emplace("Array", context["Array"]);
            context.get_function("~").push_front(f);
        }

    }

}
