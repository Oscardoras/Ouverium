#include <algorithm>
#include <iostream>
#include <fstream>

#include "../Interpreter.hpp"


namespace Math {

    std::shared_ptr<Expression> a() {
        auto a = std::make_shared<Symbol>();
        a->name = "a";
        return a;
    }
    std::shared_ptr<Expression> ab() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }

    Reference logical_not(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        
        if (a->type == Object::Bool) return Reference(context.newObject(!a->data.b));
        else throw FunctionArgumentsError();
    }

    Reference logical_and(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        if (a->type == Object::Bool && b->type == Object::Bool)
            return Reference(context.newObject(a->data.b && b->data.b));
        else throw FunctionArgumentsError();
    }

    Reference logical_or(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        if (a->type == Object::Bool && b->type == Object::Bool)
            return Reference(context.newObject(a->data.b || b->data.b));
        else throw FunctionArgumentsError();
    }

    Reference addition(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i + b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f + b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference opposite(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
            
        if (a->type == Object::Int)
            return Reference(context.newObject(-a->data.i));
        else if (a->type == Object::Float)
            return Reference(context.newObject(-a->data.f));
        else throw FunctionArgumentsError();
    }

    Reference substraction(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i - b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f - b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference multiplication(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i * b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f * b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference division(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int && b->data.i != 0)
            return Reference(context.newObject(a->data.i / b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float && b->data.f != 0)
            return Reference(context.newObject(a->data.f / b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference modulo(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i % b->data.i));
        else throw FunctionArgumentsError();
    }

    Reference strictly_inf(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i < b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f < b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference strictly_sup(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i > b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f > b->data.f));
        else throw FunctionArgumentsError();
    }
    
    Reference inf_equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i <= b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f <= b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference sup_equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i >= b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f >= b->data.f));
        else throw FunctionArgumentsError();
    }

    Reference increment(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
            
        if (a->type == Object::Int) {
            a->data.i++;
            return context.getSymbol("a");
        } else throw FunctionArgumentsError();
    }

    Reference decrement(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
            
        if (a->type == Object::Int) {
            a->data.i--;
            return context.getSymbol("a");
        } else throw FunctionArgumentsError();
    }

    Reference add(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            a->data.i += b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float)
            a->data.f += b->data.f;
        else throw FunctionArgumentsError();

        return Reference(context.newObject());
    }

    Reference remove(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            a->data.i -= b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float)
            a->data.f -= b->data.f;
        else throw FunctionArgumentsError();

        return Reference(context.newObject());
    }

    Reference mutiply(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            a->data.i *= b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float)
            a->data.f *= b->data.f;
        else throw FunctionArgumentsError();

        return Reference(context.newObject());
    }

    Reference divide(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int && b->data.i != 0)
            a->data.i /= b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float && b->data.i != 0)
            a->data.f /= b->data.f;
        else throw FunctionArgumentsError();

        return Reference(context.newObject());
    }

    void initiate(Context & context) {
        context.getSymbol("!").toObject(context)->functions.push_front(new SystemFunction(a(), logical_not));
        context.getSymbol("&").toObject(context)->functions.push_front(new SystemFunction(ab(), logical_and));
        context.getSymbol("|").toObject(context)->functions.push_front(new SystemFunction(ab(), logical_or));
        context.getSymbol("+").toObject(context)->functions.push_front(new SystemFunction(ab(), addition));
        context.getSymbol("-").toObject(context)->functions.push_front(new SystemFunction(a(), opposite));
        context.getSymbol("-").toObject(context)->functions.push_front(new SystemFunction(ab(), substraction));
        context.getSymbol("*").toObject(context)->functions.push_front(new SystemFunction(ab(), multiplication));
        context.getSymbol("/").toObject(context)->functions.push_front(new SystemFunction(ab(), division));
        context.getSymbol("%").toObject(context)->functions.push_front(new SystemFunction(ab(), modulo));
        context.getSymbol("<").toObject(context)->functions.push_front(new SystemFunction(ab(), strictly_inf));
        context.getSymbol(">").toObject(context)->functions.push_front(new SystemFunction(ab(), strictly_sup));
        context.getSymbol("<=").toObject(context)->functions.push_front(new SystemFunction(ab(), inf_equals));
        context.getSymbol(">=").toObject(context)->functions.push_front(new SystemFunction(ab(), sup_equals));
        context.getSymbol("++").toObject(context)->functions.push_front(new SystemFunction(a(), increment));
        context.getSymbol("--").toObject(context)->functions.push_front(new SystemFunction(a(), decrement));
        context.getSymbol(":+").toObject(context)->functions.push_front(new SystemFunction(ab(), add));
        context.getSymbol(":-").toObject(context)->functions.push_front(new SystemFunction(ab(), remove));
        context.getSymbol(":*").toObject(context)->functions.push_front(new SystemFunction(ab(), mutiply));
        context.getSymbol(":/").toObject(context)->functions.push_front(new SystemFunction(ab(), divide));
    }

}