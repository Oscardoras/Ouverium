#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>
#include <ostream>

#include "../Types.hpp"


namespace Interpreter {

    class Object;

    class Data : protected std::variant<Object*, char, FLOAT, INT, bool> {

    public:

        using std::variant<Object*, char, FLOAT, INT, bool>::variant;
        using std::variant<Object*, char, FLOAT, INT, bool>::operator=;

        friend bool operator==(Data const& a, Data const& b) {
            return static_cast<std::variant<Object*, char, FLOAT, INT, bool>const&>(a) == static_cast<std::variant<Object*, char, FLOAT, INT, bool>const&>(b);
        }

        template<typename T>
        T& get() {
            return std::get<T>(*this);
        }
        template<typename T>
        T const& get() const {
            return std::get<T>(*this);
        }
        template<typename T>
        friend T* get_if(Data* data) {
            return std::get_if<T>(data);
        }
        template<typename T>
        friend T const* get_if(Data const* data) {
            return std::get_if<T>(data);
        }
        template<typename T>
        bool is() const {
            return std::holds_alternative<T>(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, Data const& data);

    };

}


#endif
