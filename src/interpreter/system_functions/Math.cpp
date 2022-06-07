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
    std::shared_ptr<Expression> a_b() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "b";
        b->function = function_name;
        b->object = std::make_shared<Tuple>();
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

        if (a->type == Object::Bool)
            if (a->data.b) {
                auto r = Interpreter::callFunction(*context.getParent(), b->functions, std::make_shared<Tuple>(), nullptr).toObject(context);
                if (r->type == Object::Bool)
                    return Reference(context.newObject(r->data.b));
                else throw FunctionArgumentsError();
            } else return Reference(context.newObject(false));
        else throw FunctionArgumentsError();
    }

    Reference logical_or(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        if (a->type == Object::Bool)
            if (!a->data.b) {
                auto r = Interpreter::callFunction(*context.getParent(), b->functions, std::make_shared<Tuple>(), nullptr).toObject(context);
                if (r->type == Object::Bool)
                    return Reference(context.newObject(r->data.b));
                else throw FunctionArgumentsError();
            } else return Reference(context.newObject(true));
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
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.newObject(a->data.c < b->data.c));
        else throw FunctionArgumentsError();
    }

    Reference strictly_sup(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i > b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f > b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.newObject(a->data.c > b->data.c));
        else throw FunctionArgumentsError();
    }
    
    Reference inf_equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i <= b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f <= b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.newObject(a->data.c <= b->data.c));
        else throw FunctionArgumentsError();
    }

    Reference sup_equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.newObject(a->data.i >= b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.newObject(a->data.f >= b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.newObject(a->data.c >= b->data.c));
        else throw FunctionArgumentsError();
    }

    Reference increment(FunctionContext & context) {
        auto a = context.getSymbol("a");
            
        if (a.getReference()->type == Object::Int) {
            a.getReference() = context.newObject(a.getReference()->data.i+1);
            return a;
        } else throw FunctionArgumentsError();
    }

    Reference decrement(FunctionContext & context) {
        auto a = context.getSymbol("a");
            
        if (a.getReference()->type == Object::Int) {
            a.getReference() = context.newObject(a.getReference()->data.i-1);
            return a;
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
        context.getSymbol("&").toObject(context)->functions.push_front(new SystemFunction(a_b(), logical_and));
        context.getSymbol("|").toObject(context)->functions.push_front(new SystemFunction(a_b(), logical_or));
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