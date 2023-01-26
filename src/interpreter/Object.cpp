#include <exception>

#include "Interpreter.hpp"


namespace Interpreter {

    Object::Object(size_t tuple_size) {
        if (tuple_size > 0) {
            array = (Object::ArrayElement *) std::malloc(sizeof(Object::ArrayElement) * (tuple_size+2));
            array[0].c = tuple_size;
            array[1].c = tuple_size;
        }
    }

    Object::~Object() {
        if (array != nullptr)
            delete[] array;
    }

    Object*& Object::get_property(std::string name, Context & context) {
        auto & field = properties[name];
        if (field == nullptr) field = context.new_object();
        return field;
    }

    std::string Object::to_string() const {
        if (array != nullptr && array[0].c > 0) {
            std::string str;

            for (size_t i = 0; i < array[1].c+2; i++) {
                auto d = array[i].o;
                if (d.type == Data::Type::Char) str.push_back(d.c);
                else throw std::exception();
            }

            return str;
        } else throw std::exception();
    }

}
