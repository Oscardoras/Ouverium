#include <algorithm>
#include <iostream>

#include "Interpreter.hpp"


namespace SystemFunctions {

    std::shared_ptr<Expression> separator() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference separator(FunctionContext & context) {
        return Reference(context.getSymbol("b"));
    }

    std::shared_ptr<Expression> if_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto condition = std::make_shared<Symbol>();
        condition->name = "condition";
        tuple->objects.push_back(condition);

        auto block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        return tuple;
    }
    Reference if_statement(FunctionContext & context) {
        auto condition = context.getSymbol("condition").toObject(context);

        if (condition->type == Object::Boolean)
            if (condition->data.b) {
                auto parent = context.getParent();
                auto functions = context.getSymbol("block").toObject(context)->functions;
                return Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>());
            } else return Reference(context.addObject(new Object()));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> if_else_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto condition = std::make_shared<Symbol>();
        condition->name = "condition";
        tuple->objects.push_back(condition);

        auto block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        auto else_s = std::make_shared<Symbol>();
        else_s->name = "else_s";
        tuple->objects.push_back(else_s);

        auto alternative = std::make_shared<FunctionCall>();
        function_name = std::make_shared<Symbol>();
        function_name->name = "alternative";
        alternative->function = function_name;
        alternative->object = std::make_shared<Tuple>();
        tuple->objects.push_back(alternative);

        return tuple;
    }
    Reference if_else_statement(FunctionContext & context) {
        auto condition = context.getSymbol("condition").toObject(context);

        if (condition->type == Object::Boolean)
            if (condition->data.b) {
                auto parent = context.getParent();
                auto functions = context.getSymbol("block").toObject(context)->functions;
                return Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>());
            } else {
                auto parent = context.getParent();
                auto functions = context.getSymbol("alternative").toObject(context)->functions;
                return Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>());
            }
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> else_statement() {
        return std::make_shared<Tuple>();
    }
    Reference else_statement(FunctionContext & context) {
        throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> while_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto condition = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "condition";
        condition->function = function_name;
        condition->object = std::make_shared<Tuple>();
        tuple->objects.push_back(condition);

        auto block = std::make_shared<FunctionCall>();
        function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        return tuple;
    }
    Reference while_statement(FunctionContext & context) {
        auto parent = context.getParent();
        Reference result;

        while (true) {
            auto functions = context.getSymbol("condition").toObject(context)->functions;
            auto condition = Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>()).toObject(context);

            if (condition->type == Object::Boolean) {
                if (condition->data.b) {
                    functions = context.getSymbol("block").toObject(context)->functions;
                    result = Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>());
                } else break;
            } else throw FunctionArgumentsError();
        }
        
        if (result.type == Reference::Pointer && result.pointer == nullptr)
            return Reference(context.addObject(new Object()));
        else return result;
    }

    std::shared_ptr<Expression> copy() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";

        return object;
    }
    Reference copy(FunctionContext & context) {
        auto object = context.getSymbol("object").toObject(context);
        return Reference(context.addObject(new Object(*object)));
    }

    std::shared_ptr<Expression> assign() {
        auto tuple = std::make_shared<Tuple>();

        auto var = std::make_shared<Symbol>();
        var->name = "var";
        tuple->objects.push_back(var);

        auto object = std::make_shared<Symbol>();
        object->name = "object";
        tuple->objects.push_back(object);

        return tuple;
    }
    Reference assign(FunctionContext & context) {
        auto var = context.getSymbol("var");
        auto object = context.getSymbol("object").toObject(context);

        if (var.isReference())
            var.getReference() = object;

        return var;
    }

    std::shared_ptr<Expression> function_definition() {
        auto tuple = std::make_shared<Tuple>();

        auto var = std::make_shared<Symbol>();
        var->name = "var";
        tuple->objects.push_back(var);

        auto object = std::make_shared<Symbol>();
        object->name = "object";
        tuple->objects.push_back(object);

        return tuple;
    }
    Reference function_definition(FunctionContext & context) {
        auto var = context.getSymbol("var").toObject(context);
        auto object = context.getSymbol("object").toObject(context);

        for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++) {
            if ((*it)->type == Function::Custom) {
                var->functions.push_front(new CustomFunction(*((CustomFunction*) *it)));
            } else var->functions.push_front(new SystemFunction(*((SystemFunction*) *it)));
        }

        return context.getSymbol("var");
    }

    bool equals(Object* a, Object* b) {
        if (a->type == b->type) {
            for (auto const& element : a->fields) {
                auto it = b->fields.find(element.first);
                if (it != b->fields.end()) {
                    if (!equals(element.second, it->second))
                        return false;
                } else return false;
            }

            auto ita = a->functions.begin();
            auto itb = b->functions.begin();
            while (ita != a->functions.end()) {
                if (itb != b->functions.end()) {
                    if ((*ita)->type == Function::Custom) {
                        if (((CustomFunction*) *ita)->pointer != ((CustomFunction*) *itb)->pointer)
                            return false;
                    } else
                        if (((SystemFunction*) *ita)->pointer != ((SystemFunction*) *itb)->pointer)
                            return false;
                } else return false;

                ita++;
                itb++;
            }

            if (a->type >= 0) {
                for (long i = 1; i <= a->type; i++) {
                    if (!equals(a->data.a[i].o, b->data.a[i].o))
                        return false;
                }
                return true;
            } else if (a->type == Object::Boolean)
                return a->data.b == b->data.b;
            else if (a->type == Object::Integer)
                return a->data.i == b->data.i;
            else if (a->type == Object::Float)
                return a->data.f == b->data.f;
            else if (a->type == Object::Char)
                return a->data.c == b->data.c;
            else return true;
        } else return false;
    }

    std::shared_ptr<Expression> equals() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
        
        return Reference(context.addObject(new Object(equals(a, b))));
    }

    std::shared_ptr<Expression> not_equals() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference not_equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
        
        return Reference(context.addObject(new Object(!equals(a, b))));
    }

    std::shared_ptr<Expression> check_pointers() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference check_pointers(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        return Reference(context.addObject(new Object(a == b)));
    }

    std::shared_ptr<Expression> logical_not() {
        auto a = std::make_shared<Symbol>();
        a->name = "a";
        return a;
    }
    Reference logical_not(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        
        if (a->type == Object::Boolean) return Reference(context.addObject(new Object(!a->data.b)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> logical_and() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference logical_and(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        if (a->type == Object::Boolean && b->type == Object::Boolean)
            return Reference(context.addObject(new Object(a->data.b && b->data.b)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> logical_or() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference logical_or(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        if (a->type == Object::Boolean && b->type == Object::Boolean)
            return Reference(context.addObject(new Object(a->data.b || b->data.b)));
        else throw FunctionArgumentsError();
    }

    void print(Object* object) {
        if (object->type == Object::Boolean) std::cout << object->data.b;
        else if (object->type == Object::Integer) std::cout << object->data.i;
        else if (object->type == Object::Float) std::cout << object->data.f;
        else if (object->type == Object::Char) std::cout << object->data.c;
        else if (object->type > 0)
            for (long i = 1; i <= object->type; i++)
                print(object->data.a[i].o);
    }

    std::shared_ptr<Expression> print() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";
        return object;
    }
    Reference print(FunctionContext & context) {
        auto object = context.getSymbol("object").toObject(context);

        print(object);

        return Reference(context.addObject(new Object()));
    }

    std::shared_ptr<Expression> addition() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference addition(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Integer && b->type == Object::Integer)
            return Reference(context.addObject(new Object(a->data.i + b->data.i)));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.addObject(new Object(a->data.f + b->data.f)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> opposite() {
        auto a = std::make_shared<Symbol>();
        a->name = "a";
        return a;
    }
    Reference opposite(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
            
        if (a->type == Object::Integer)
            return Reference(context.addObject(new Object(-a->data.i)));
        else if (a->type == Object::Float)
            return Reference(context.addObject(new Object(-a->data.f)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> substraction() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference substraction(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Integer && b->type == Object::Integer)
            return Reference(context.addObject(new Object(a->data.i - b->data.i)));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.addObject(new Object(a->data.f - b->data.f)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> multiplication() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference multiplication(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Integer && b->type == Object::Integer)
            return Reference(context.addObject(new Object(a->data.i * b->data.i)));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.addObject(new Object(a->data.f * b->data.f)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> division() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference division(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Integer && b->type == Object::Integer)
            return Reference(context.addObject(new Object(a->data.i / b->data.i)));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.addObject(new Object(a->data.f / b->data.f)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> modulo() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference modulo(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Integer && b->type == Object::Integer)
            return Reference(context.addObject(new Object(a->data.i / b->data.i)));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.addObject(new Object(a->data.f / b->data.f)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> get_array_size() {
        auto array = std::make_shared<Symbol>();
        array->name = "array";
        return array;
    }
    Reference get_array_size(FunctionContext & context) {
        auto array = context.getSymbol("array").toObject(context);
        
        if (array->type >= 0)
            return Reference(context.addObject(new Object((long) array->type)));
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> get_array_element() {
        auto tuple = std::make_shared<Tuple>();

        auto array = std::make_shared<Symbol>();
        array->name = "array";
        tuple->objects.push_back(array);

        auto i = std::make_shared<Symbol>();
        i->name = "i";
        tuple->objects.push_back(i);

        return tuple;
    }
    Reference get_array_element(FunctionContext & context) {
        auto array = context.getSymbol("array").toObject(context);
        auto i = context.getSymbol("i").toObject(context);

        if (array->type > 0 && i->type == Object::Integer)
            return Reference(array, i->data.i);
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> get_array_capacity() {
        auto array = std::make_shared<Symbol>();
        array->name = "array";
        return array;
    }
    Reference get_array_capacity(FunctionContext & context) {
        auto array = context.getSymbol("array").toObject(context);

        if (array->type == 0) return new Object((long) 0);
        else if (array->type > 0) return new Object((long) array->data.a[0].c);
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> set_array_capacity() {
        auto tuple = std::make_shared<Tuple>();

        auto array = std::make_shared<Symbol>();
        array->name = "array";
        tuple->objects.push_back(array);

        auto capacity = std::make_shared<Symbol>();
        capacity->name = "capacity";
        tuple->objects.push_back(capacity);

        return tuple;
    }
    Reference set_array_capacity(FunctionContext & context) {
        auto array = context.getSymbol("array").toObject(context);
        auto capacity = context.getSymbol("capacity").toObject(context);

        if (capacity->type == Object::Integer && capacity->data.i >= 0) {
            if (array->type <= 0) {
                if (capacity->data.i > 0) {
                    array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * (capacity->data.i+1));
                    array->data.a[0].c = capacity->data.i;
                }
                array->type = 0;
            } else if (capacity->data.i != (long) array->data.a[0].c) {
                array->data.a[0].c = capacity->data.i;
                array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
            }

            return context.addObject(new Object());
        } else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> add_array_element() {
        auto tuple = std::make_shared<Tuple>();

        auto array = std::make_shared<Symbol>();
        array->name = "array";
        tuple->objects.push_back(array);

        auto element = std::make_shared<Symbol>();
        element->name = "element";
        tuple->objects.push_back(element);

        return tuple;
    }
    Reference add_array_element(FunctionContext & context) {
        auto array = context.getSymbol("array").toObject(context);
        auto element = context.getSymbol("element").toObject(context);

        if (array->type <= 0) {
            array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * 2);
            array->data.a[0].c = 1;
        } else if ((long) array->data.a[0].c <= array->type) {
            array->data.a[0].c *= 2;
            array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
        }

        array->type++;
        array->data.a[array->type].o = element;

        return Reference(array, array->type);
    }

    std::shared_ptr<Expression> remove_array_element() {
        auto array = std::make_shared<Symbol>();
        array->name = "array";
        return array;
    }
    Reference remove_array_element(FunctionContext & context) {
        auto array = context.getSymbol("array").toObject(context);

        if (array->type > 0) {
            array->type--;
            return Reference(array, array->type+1);
        } else throw FunctionArgumentsError();
    }

}