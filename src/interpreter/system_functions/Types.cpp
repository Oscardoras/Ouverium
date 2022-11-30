#include "Array.hpp"
#include "Types.hpp"


namespace Interpreter {

    namespace Types {

        bool check_type(Context & context, Object* object, Object* type) {
            if (type == context.get_symbol("Char").to_object(context)) return object->type == Object::Char;
            else if (type == context.get_symbol("Int").to_object(context)) return object->type == Object::Int;
            else if (type == context.get_symbol("Float").to_object(context)) return object->type == Object::Float;
            else if (type == context.get_symbol("Bool").to_object(context)) return object->type == Object::Bool;
            else if (type == context.get_symbol("Array").to_object(context)) return object->type >= 0;
            else {
                auto array = object->get_property("_types", context);
                for (long i = 1; i <= array->type; i++) {
                    if (array->data.a[i].o == type)
                        return true;
                }
                return false;
            }
        }

        std::shared_ptr<Expression> getset_type() {
            auto tuple = std::make_shared<Tuple>();

            auto object = std::make_shared<Symbol>();
            object->name = "object";
            tuple->objects.push_back(object);

            auto type = std::make_shared<Symbol>();
            type->name = "type";
            tuple->objects.push_back(type);

            return tuple;
        }

        Reference is_type(FunctionContext & context) {
            auto object = context.get_symbol("object").to_object(context);
            auto type = context.get_symbol("type").to_object(context);

            return Reference(context.new_object(check_type(context, object, type)));
        }

        Reference set_type(FunctionContext & context) {
            auto object = context.get_symbol("object");
            auto type = context.get_symbol("type").to_object(context);

            auto array = object.to_object(context)->get_property("_types", context);

            FunctionContext function_context(context, context.position);
            function_context.get_symbol("array").get_reference() = array;
            function_context.get_symbol("element").get_reference() = type;

            Array::add_array_element(function_context);

            return object;
        }

        void init(Context & context) {
            auto parameters = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("object"),
                std::make_shared<Symbol>("type")
            });
            auto f = std::make_unique<SystemFunction>(parameters, is_type);
            f->extern_symbols["Char"] = context.get_symbol("Char");
            f->extern_symbols["Int"] = context.get_symbol("Int");
            f->extern_symbols["Float"] = context.get_symbol("Float");
            f->extern_symbols["Bool"] = context.get_symbol("Bool");
            f->extern_symbols["Array"] = context.get_symbol("Array");
            context.get_symbol("~").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(*f));
            context.get_symbol("is").to_object(context)->functions.push_front(std::move(f));

            context.get_symbol(":~").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(parameters, set_type));
        }

    }

}
