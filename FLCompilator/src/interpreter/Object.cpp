#include "Context.hpp"


Object::Object() {
    function = nullptr;
    type = None;
    references = 0;
}

Object::Object(Object const& object) {
    if (object.function != nullptr) {
        if (object.function->type == Function::Custom) {
            function = new CustomFunction(((CustomFunction*) object.function)->pointer);
            ((CustomFunction*) function)->objects = ((CustomFunction*) object.function)->objects;
        } else {
            function = new SystemFunction(((SystemFunction*) object.function)->pointer);
        }
    } else function = nullptr;

    fields = object.fields;

    type = object.type;
    if (type == Float) data.f = object.data.f;
    else if (type == Integer) data.i = object.data.i;
    else if (type == Boolean) data.b = object.data.b;
    else if (type > 0) {
        data.a = new Object::Data::ArrayElement[type+1];
        data.a[0].c = (long) object.data.a[0].c;
        for (long i = 1; i <= type; i++)
            data.a[i].o = object.data.a[i].o;
    }

    references = 0;
}

Object::Object(bool const& b) {
    function = nullptr;
    type = Boolean;
    data.b = b;
    references = 0;
}

Object::Object(long const& i) {
    function = nullptr;
    type = Integer;
    data.i = i;
    references = 0;
}

Object::Object(double const& f) {
    function = nullptr;
    type = Float;
    data.f = f;
    references = 0;
}

Object::Object(size_t const& tuple_size) {
    function = nullptr;
    type = (long) tuple_size;
    data.a = new Object::Data::ArrayElement[type+1];
    data.a[0].c = type;
    references = 0;
}

Object::~Object() {
    if (references == 0) {
        for (auto & element : fields)
            if (element.second->references == 0) delete element.second;
        if (function != nullptr) {
            if (function->type == Function::Custom) {
                for (auto & element : ((CustomFunction*) function)->objects)
                    if (element.second->references == 0) delete element.second;
            }
            delete function;
        }
        if (type > 0) {
            for (long i = 1; i <= type; i++)
                if (data.a[i].o->references == 0) delete data.a[i].o;
            delete[] data.a;
        }
    } else {
        for (auto & element : fields)
            element.second->removeReference();
        if (function != nullptr) {
            if (function->type == Function::Custom) {
                for (auto & element : ((CustomFunction*) function)->objects)
                    element.second->removeReference();
            }
            delete function;
        }
        if (type > 0) {
            for (long i = 1; i <= type; i++)
                data.a[i].o->removeReference();
            delete[] data.a;
        }
    }
}

void Object::addReference() {
    if (references == 0) {
        for (auto & element : fields)
            element.second->addReference();
        if (function != nullptr && function->type == Function::Custom)
            for (auto & element : ((CustomFunction*) function)->objects)
                element.second->addReference();
        if (type > 0)
            for (long i = 1; i <= type; i++)
                data.a[i].o->addReference();
    }
    references++;
}

void Object::removeReference() {
    if (references <= 0) throw "Negative references";
    
    references--;
    if (references == 0)
        delete this;
}