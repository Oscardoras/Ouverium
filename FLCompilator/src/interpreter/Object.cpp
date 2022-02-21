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
        } else function = new SystemFunction(((SystemFunction*) object.function)->pointer);
    } else function = nullptr;

    fields = object.fields;

    type = object.type;
    if (type == Float) data.f = object.data.f;
    else if (type == Integer) data.i = object.data.i;
    else if (type == Boolean) data.b = object.data.b;
    else if (type >= 0) {
        data.tuple = new Object*[type];
        for (auto i = 0; i < type; i++)
            data.tuple[i] = object.data.tuple[i];
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
    type = (int) tuple_size;
    data.tuple = new Object*[tuple_size];
    references = 0;
}

Object::~Object() {
    if (type != Deleting) {
        if (references == 0) {
            for (auto & element : fields)
                if (element.second != nullptr && element.second->references == 0) delete element.second;
            if (function != nullptr) {
                if (function->type == Function::Custom) {
                    for (auto & element : ((CustomFunction*) function)->objects)
                        if (element.second != nullptr && element.second->references == 0) delete element.second;
                }
                delete function;
            }
            if (type > 0) {
                for (auto i = 0; i < type; i++)
                    if (data.tuple[i] != nullptr && data.tuple[i]->references == 0) delete data.tuple[i];
                delete[] data.tuple;
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
                for (auto i = 0; i < type; i++)
                    data.tuple[i]->removeReference();
                delete[] data.tuple;
            }
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
            for (auto i = 0; i < type; i++)
                data.tuple[i]->addReference();
    }
    references++;
}

void Object::removeReference() {
    if (references <= 0) throw "Negative references";
    
    references--;
    if (references == 0)
        delete this;
}