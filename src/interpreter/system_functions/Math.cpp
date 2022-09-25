#include <algorithm>
#include <iostream>
#include <fstream>

#include "Math.hpp"


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
        auto a = context.get_symbol("a").to_object(context);
        
        if (a->type == Object::Bool) return Reference(context.new_object(!a->data.b));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference logical_and(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);

        if (a->type == Object::Bool)
            if (a->data.b) {
                auto r = Interpreter::call_function(*context.get_parent(), nullptr, b->functions, std::make_shared<Tuple>()).to_object(context);
                if (r->type == Object::Bool)
                    return Reference(context.new_object(r->data.b));
                else throw Interpreter::FunctionArgumentsError();
            } else return Reference(context.new_object(false));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference logical_or(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);

        if (a->type == Object::Bool)
            if (!a->data.b) {
                auto r = Interpreter::call_function(*context.get_parent(), nullptr, b->functions, std::make_shared<Tuple>()).to_object(context);
                if (r->type == Object::Bool)
                    return Reference(context.new_object(r->data.b));
                else throw Interpreter::FunctionArgumentsError();
            } else return Reference(context.new_object(true));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference addition(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i + b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f + b->data.f));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference opposite(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
            
        if (a->type == Object::Int)
            return Reference(context.new_object(-a->data.i));
        else if (a->type == Object::Float)
            return Reference(context.new_object(-a->data.f));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference substraction(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i - b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f - b->data.f));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference multiplication(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i * b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f * b->data.f));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference division(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int && b->data.i != 0)
            return Reference(context.new_object(a->data.i / b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float && b->data.f != 0)
            return Reference(context.new_object(a->data.f / b->data.f));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference modulo(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i % b->data.i));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference strictly_inf(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i < b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f < b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.new_object(a->data.c < b->data.c));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference strictly_sup(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i > b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f > b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.new_object(a->data.c > b->data.c));
        else throw Interpreter::FunctionArgumentsError();
    }
    
    Reference inf_equals(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i <= b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f <= b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.new_object(a->data.c <= b->data.c));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference sup_equals(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            return Reference(context.new_object(a->data.i >= b->data.i));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.new_object(a->data.f >= b->data.f));
        else if (a->type == Object::Char && b->type == Object::Char)
            return Reference(context.new_object(a->data.c >= b->data.c));
        else throw Interpreter::FunctionArgumentsError();
    }

    Reference increment(FunctionContext & context) {
        auto a = context.get_symbol("a");
            
        if (a.get_reference()->type == Object::Int) {
            a.get_reference() = context.new_object(a.get_reference()->data.i+1);
            return a;
        } else throw Interpreter::FunctionArgumentsError();
    }

    Reference decrement(FunctionContext & context) {
        auto a = context.get_symbol("a");
            
        if (a.get_reference()->type == Object::Int) {
            a.get_reference() = context.new_object(a.get_reference()->data.i-1);
            return a;
        } else throw Interpreter::FunctionArgumentsError();
    }

    Reference add(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            a->data.i += b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float)
            a->data.f += b->data.f;
        else throw Interpreter::FunctionArgumentsError();

        return Reference(context.new_object());
    }

    Reference remove(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            a->data.i -= b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float)
            a->data.f -= b->data.f;
        else throw Interpreter::FunctionArgumentsError();

        return Reference(context.new_object());
    }

    Reference mutiply(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int)
            a->data.i *= b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float)
            a->data.f *= b->data.f;
        else throw Interpreter::FunctionArgumentsError();

        return Reference(context.new_object());
    }

    Reference divide(FunctionContext & context) {
        auto a = context.get_symbol("a").to_object(context);
        auto b = context.get_symbol("b").to_object(context);
            
        if (a->type == Object::Int && b->type == Object::Int && b->data.i != 0)
            a->data.i /= b->data.i;
        else if (a->type == Object::Float && b->type == Object::Float && b->data.i != 0)
            a->data.f /= b->data.f;
        else throw Interpreter::FunctionArgumentsError();

        return Reference(context.new_object());
    }

    void init(Context & context) {
        context.get_symbol("!").to_object(context)->functions.push_front(new SystemFunction(a(), logical_not));
        context.get_symbol("&").to_object(context)->functions.push_front(new SystemFunction(a_b(), logical_and));
        context.get_symbol("|").to_object(context)->functions.push_front(new SystemFunction(a_b(), logical_or));
        context.get_symbol("+").to_object(context)->functions.push_front(new SystemFunction(ab(), addition));
        context.get_symbol("-").to_object(context)->functions.push_front(new SystemFunction(a(), opposite));
        context.get_symbol("-").to_object(context)->functions.push_front(new SystemFunction(ab(), substraction));
        context.get_symbol("*").to_object(context)->functions.push_front(new SystemFunction(ab(), multiplication));
        context.get_symbol("/").to_object(context)->functions.push_front(new SystemFunction(ab(), division));
        context.get_symbol("%").to_object(context)->functions.push_front(new SystemFunction(ab(), modulo));
        context.get_symbol("<").to_object(context)->functions.push_front(new SystemFunction(ab(), strictly_inf));
        context.get_symbol(">").to_object(context)->functions.push_front(new SystemFunction(ab(), strictly_sup));
        context.get_symbol("<=").to_object(context)->functions.push_front(new SystemFunction(ab(), inf_equals));
        context.get_symbol(">=").to_object(context)->functions.push_front(new SystemFunction(ab(), sup_equals));
        context.get_symbol("++").to_object(context)->functions.push_front(new SystemFunction(a(), increment));
        context.get_symbol("--").to_object(context)->functions.push_front(new SystemFunction(a(), decrement));
        context.get_symbol(":+").to_object(context)->functions.push_front(new SystemFunction(ab(), add));
        context.get_symbol(":-").to_object(context)->functions.push_front(new SystemFunction(ab(), remove));
        context.get_symbol(":*").to_object(context)->functions.push_front(new SystemFunction(ab(), mutiply));
        context.get_symbol(":/").to_object(context)->functions.push_front(new SystemFunction(ab(), divide));
    }

}
