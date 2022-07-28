#include "../Interpreter.hpp"


namespace Types {

    std::shared_ptr<Expression> is_type() {
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

        if (type == context.get_symbol("Char").to_object(context)) return Reference(context.new_object(object->type == Object::Char));
        else if (type == context.get_symbol("Int").to_object(context)) return Reference(context.new_object(object->type == Object::Int));
        else if (type == context.get_symbol("Float").to_object(context)) return Reference(context.new_object(object->type == Object::Float));
        else if (type == context.get_symbol("Bool").to_object(context)) return Reference(context.new_object(object->type == Object::Bool));
        else if (type == context.get_symbol("Array").to_object(context)) return Reference(context.new_object(object->type >= 0));
        else throw Interpreter::FunctionArgumentsError();
    }

    void initiate(Context & context) {
        auto f = new SystemFunction(is_type(), is_type);
        f->extern_symbols["Char"] = context.get_symbol("Char");
        f->extern_symbols["Int"] = context.get_symbol("Int");
        f->extern_symbols["Float"] = context.get_symbol("Float");
        f->extern_symbols["Bool"] = context.get_symbol("Bool");
        f->extern_symbols["Array"] = context.get_symbol("Array");
        context.get_symbol("is").to_object(context)->functions.push_front(f);

        f = new SystemFunction(is_type(), is_type);
        f->extern_symbols["Char"] = context.get_symbol("Char");
        f->extern_symbols["Int"] = context.get_symbol("Int");
        f->extern_symbols["Float"] = context.get_symbol("Float");
        f->extern_symbols["Bool"] = context.get_symbol("Bool");
        f->extern_symbols["Array"] = context.get_symbol("Array");
        context.get_symbol("~").to_object(context)->functions.push_front(f);
    }

}
