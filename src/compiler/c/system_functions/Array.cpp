#include <algorithm>
#include <iostream>
#include <fstream>

#include "Array.hpp"


namespace Array {

    std::shared_ptr<Expression> get_array_size() {
        auto array = std::make_shared<Symbol>();
        array->name = "array";
        return array;
    }
    Reference get_array_size(FunctionContext & context) {
        auto array = context.get_symbol("array").to_object(context);
        
        if (array->type >= 0)
            return Reference(context.new_object((long) array->type));
        else throw Interpreter::FunctionArgumentsError();
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
        auto array = context.get_symbol("array").to_object(context);
        auto i = context.get_symbol("i").to_object(context);

        if (i->type == Object::Int && i->data.i >= 0 && i->data.i < array->type)
            return Reference(array, i->data.i);
        else throw Interpreter::FunctionArgumentsError();
    }

    std::shared_ptr<Expression> get_array_capacity() {
        auto array = std::make_shared<Symbol>();
        array->name = "array";
        return array;
    }
    Reference get_array_capacity(FunctionContext & context) {
        auto array = context.get_symbol("array").to_object(context);

        if (array->type == 0) return context.new_object((long) 0);
        else if (array->type > 0) return context.new_object((long) array->data.a[0].c);
        else throw Interpreter::FunctionArgumentsError();
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
        auto array = context.get_symbol("array").to_object(context);
        auto capacity = context.get_symbol("capacity").to_object(context);

        if (capacity->type == Object::Int && capacity->data.i >= 0) {
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

            return context.new_object();
        } else throw Interpreter::FunctionArgumentsError();
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
        auto array = context.get_symbol("array").to_object(context);
        auto element = context.get_symbol("element").to_object(context);

        if (array->type <= 0) {
            array->type = 0;
            array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * 2);
            array->data.a[0].c = 1;
        } else if ((long) array->data.a[0].c <= array->type) {
            array->data.a[0].c *= 2;
            array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
        }

        array->type++;
        array->data.a[array->type].o = element;

        return Reference(array, array->type-1);
    }

    std::shared_ptr<Expression> remove_array_element() {
        auto array = std::make_shared<Symbol>();
        array->name = "array";
        return array;
    }
    Reference remove_array_element(FunctionContext & context) {
        auto array = context.get_symbol("array").to_object(context);

        if (array->type > 0) {
            array->type--;
            return Reference(array, array->type);
        } else throw Interpreter::FunctionArgumentsError();
    }

    void init(Context & context) {
        auto array = context.get_symbol("Array").to_object(context);

        array->get_property("lenght", context)->functions.push_front(new SystemFunction(get_array_size(), get_array_size));
        array->get_property("get_capacity", context)->functions.push_front(new SystemFunction(get_array_capacity(), get_array_capacity));
        array->get_property("set_capacity", context)->functions.push_front(new SystemFunction(set_array_capacity(), set_array_capacity));
        array->get_property("get", context)->functions.push_front(new SystemFunction(get_array_element(), get_array_element));
        array->get_property("add", context)->functions.push_front(new SystemFunction(add_array_element(), add_array_element));
        array->get_property("remove", context)->functions.push_front(new SystemFunction(remove_array_element(), remove_array_element));
    }

}
