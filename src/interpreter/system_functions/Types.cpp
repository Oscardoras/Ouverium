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
        auto object = context.getSymbol("object").toObject(context);
        auto type = context.getSymbol("type").toObject(context);

        if (type == context.getSymbol("Char").toObject(context)) return Reference(context.newObject(object->type == Object::Char));
        else if (type == context.getSymbol("Int").toObject(context)) return Reference(context.newObject(object->type == Object::Int));
        else if (type == context.getSymbol("Float").toObject(context)) return Reference(context.newObject(object->type == Object::Float));
        else if (type == context.getSymbol("Bool").toObject(context)) return Reference(context.newObject(object->type == Object::Bool));
        else if (type == context.getSymbol("Array").toObject(context)) return Reference(context.newObject(object->type >= 0));
        else throw FunctionArgumentsError();
    }

    void initiate(Context & context) {
        auto f = new SystemFunction(is_type(), is_type);
        f->externSymbols["Char"] = context.getSymbol("Char");
        f->externSymbols["Int"] = context.getSymbol("Int");
        f->externSymbols["Float"] = context.getSymbol("Float");
        f->externSymbols["Bool"] = context.getSymbol("Bool");
        f->externSymbols["Array"] = context.getSymbol("Array");
        context.getSymbol("is").toObject(context)->functions.push_front(f);

        f = new SystemFunction(is_type(), is_type);
        f->externSymbols["Char"] = context.getSymbol("Char");
        f->externSymbols["Int"] = context.getSymbol("Int");
        f->externSymbols["Float"] = context.getSymbol("Float");
        f->externSymbols["Bool"] = context.getSymbol("Bool");
        f->externSymbols["Array"] = context.getSymbol("Array");
        context.getSymbol("~").toObject(context)->functions.push_front(f);
    }

}