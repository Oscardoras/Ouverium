#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace Interpreter {

    struct Object;

    struct Data {

        enum class Type {
            Object,
            Char,
            Float,
            Int,
            Bool
        } type;

        union {
            Object* o;
            char c;
            double f;
            long i;
            bool b;
        };

        inline Data(Object* o): type(Type::Object), o(o) {}
        inline Data(char c): type(Type::Char), c(c) {}
        inline Data(double f): type(Type::Float), f(f) {}
        inline Data(long i): type(Type::Int), i(i) {}
        inline Data(bool b): type(Type::Bool), b(b) {}

    };

}


#endif
