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

    std::shared_ptr<Expression> copy() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";

        return object;
    }
    Reference copy(FunctionContext & context) {
        auto object = context.getSymbol("object").toObject(context);
        return Reference(context.addObject(new Object(*object)));
    }

    void assign1(std::vector<Object*> & cache, Reference const& var, Object* const& object) {
        if (var.type >= 0) cache.push_back(object);
        else if (var.isTuple()) {
            auto n = var.getTupleSize();
            if ((long) n == object->type)
                for (long i = 0; i < n; i++) assign1(cache, var.tuple[i], object->data.a[i+1].o);
            else throw InterpreterError();
        }
    }

    void assign2(Context & context, std::vector<Object*>::iterator & it, Reference const& var) {
        if (var.isPointerReference()) {
            *var.ptrRef = *it++;
        } else if (var.isArrayReference()) {
            auto ref = var.getArrayReference();
            *ref = *it++;
        } else if (var.isTuple()) {
            auto n = var.getTupleSize();
            for (long i = 0; i < n; i++) assign2(context, it, var.tuple[i]);
        }
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

        std::vector<Object*> cache;
        assign1(cache, var, object);
        auto it = cache.begin();
        assign2(context, it, var);
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
    Reference function_definition(FunctionContext & context) {
        auto var = context.getSymbol("var").toObject(context);
        auto object = context.getSymbol("object").toObject(context);

        for (auto it = object->functions.end(); it != object->functions.begin(); it--) {
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

            if (a->function != nullptr && b->function != nullptr) {
                if (a->function->type == b->function->type) {
                    if (a->function->type == Function::Custom) {
                        if (((CustomFunction*) a->function)->pointer != ((CustomFunction*) b->function)->pointer)
                            return false;
                        if (((CustomFunction*) a->function)->objects != ((CustomFunction*) b->function)->objects)
                            return false;
                    } else if (((SystemFunction*) a->function)->pointer != ((SystemFunction*) b->function)->pointer)
                        return false;
                } else
                    return false;
            } else if (a->function != b->function)
                return false;

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
        else throw InterpreterError();
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
        else throw InterpreterError();
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
        else throw InterpreterError();
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
    Reference addition(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
            
        if (a->type == Object::Integer && b->type == Object::Integer)
            return Reference(context.addObject(new Object(a->data.i + b->data.i)));
        else if (a->type == Object::Float && b->type == Object::Float)
            return Reference(context.addObject(new Object(a->data.f + b->data.f)));
        else throw InterpreterError();
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
    Reference substraction(FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == Object::Integer) return Reference(new Object(-tuple->data.i));
        else if (tuple->type == Object::Float) return Reference(new Object(-tuple->data.f));
        else if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i - object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f - object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference multiplication(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i * object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f * object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference division(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i / object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f / object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference modulo(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i % object2->data.i));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference get_array_size(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto object = context.getSymbol("arguments").toObject();
        
        if (object->type >= 0)
            return Reference(new Object((long) object->type));
        else throw InterpreterError();
    }

    Reference get_array_element(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto array = tuple->data.a[1].o;
            auto index = tuple->data.a[2].o;
            
            if (array->type > 0 && index->type == Object::Integer)
                return Reference(array, index->data.i);
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference get_array_capacity(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto array = context.getSymbol("arguments").toObject();

        if (array->type == 0) return new Object((long) 0);
        else if (array->type > 0) return new Object((long) array->data.a[0].c);
        else throw InterpreterError();
    }

    Reference set_array_capacity(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 3) {
            auto array = tuple->data.a[1].o;
            auto capacity = tuple->data.a[2].o;
            
            if (capacity->type == Object::Integer && capacity->data.i >= 0) {
                if (array->type <= 0) {
                    if (capacity->data.i > 0) {
                        array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * (capacity->data.i+1));
                        array->data.a[0].c = capacity->data.i;
                    }
                    array->type = 0;
                } else if (capacity->data.i != (long) array->data.a[0].c) {
                    if (capacity->data.i < array->type && array->references > 0)
                        for (int i = capacity->data.i; i <= array->type; i++)
                            context.addReference(capacity->data.a[i].o);
                    array->data.a[0].c = capacity->data.i;
                    array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
                }

                return new Object();
            } else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference add_array_element(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto array = tuple->data.a[1].o;
            auto element = tuple->data.a[2].o;
            
            if (array->type <= 0) {
                array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * 2);
                array->data.a[0].c = 1;
            } else if ((long) array->data.a[0].c <= array->type) {
                array->data.a[0].c *= 2;
                array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
            }

            array->type++;
            array->data.a[array->type].o = element;
            if (array->references > 0) context.addReference(element);

            return Reference(array, array->type);
        } else throw InterpreterError();
    }

    Reference remove_array_element(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto array = context.getSymbol("arguments").toObject();

        if (array->type > 0) {
            if (array->references > 0) context.removeReference(array->data.a[array->type].o);
            array->type--;
        } else throw InterpreterError();
    }

}