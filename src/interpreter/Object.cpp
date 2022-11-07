#include <exception>

#include "Function.hpp"
#include "Interpreter.hpp"


Object::Object(Object const& object) {
    for (auto & f : object.functions) {
        switch (f->type) {
        case Function::Custom:
            functions.push_front(std::make_unique<CustomFunction>(*f));
            break;
        case Function::System:
            functions.push_front(std::make_unique<SystemFunction>(*f));
            break;
        }
    }

    properties = object.properties;

    type = object.type;
    if (type == CPointer) data.ptr = object.data.ptr;
    else if (type == Char) data.c = object.data.c;
    else if (type == Float) data.f = object.data.f;
    else if (type == Int) data.i = object.data.i;
    else if (type == Bool) data.b = object.data.b;
    else if (type > 0) {
        data.a = (Object::Data::ArrayElement *) std::malloc(sizeof(Object::Data::ArrayElement) * (object.data.a[0].c+1));
        data.a[0].c = (long) object.data.a[0].c;
        for (long i = 1; i <= type; i++)
            data.a[i].o = object.data.a[i].o;
    }

    referenced = false;
}

Object::Object(void* ptr) {
    type = CPointer;
    data.ptr = ptr;
    referenced = false;
}

Object::Object(bool b) {
    type = Bool;
    data.b = b;
    referenced = false;
}

Object::Object(long i) {
    type = Int;
    data.i = i;
    referenced = false;
}

Object::Object(double f) {
    type = Float;
    data.f = f;
    referenced = false;
}

Object::Object(char c) {
    type = Char;
    data.c = c;
    referenced = false;
}

Object::Object(size_t tuple_size) {
    type = (long) tuple_size;
    if (type > 0) {
        data.a = (Object::Data::ArrayElement *) std::malloc(sizeof(Object::Data::ArrayElement) * (type+1));
        data.a[0].c = type;
    }
    referenced = false;
}

Object* &Object::get_property(std::string name, Context & context) {
    auto & field = properties[name];
    if (field == nullptr) field = context.new_object();
    return field;
}

std::string Object::to_string() const {
    if (type >= 0) {
        std::string str;

        for (long i = 1; i <= type; i++) {
            auto o = data.a[i].o;
            if (o->type == Object::Char) str.push_back(o->data.c);
            else throw std::exception();
        }

        return str;
    } else throw std::exception();
}
