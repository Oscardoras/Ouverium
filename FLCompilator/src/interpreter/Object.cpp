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
    if (type == Char) data.c = object.data.c;
    else if (type == Float) data.f = object.data.f;
    else if (type == Integer) data.i = object.data.i;
    else if (type == Boolean) data.b = object.data.b;
    else if (type > 0) {
        data.a = (Object::Data::ArrayElement *) std::malloc(sizeof(Object::Data::ArrayElement) * (type+1));
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

Object::Object(char const& c) {
    function = nullptr;
    type = Char;
    data.c = c;
    references = 0;
}

Object::Object(size_t const& tuple_size) {
    function = nullptr;
    type = (long) tuple_size;
    data.a = (Object::Data::ArrayElement *) std::malloc(sizeof(Object::Data::ArrayElement) * (type+1));
    data.a[0].c = type;
    references = 0;
}