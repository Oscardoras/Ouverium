#ifndef __DATA_HPP__
#define __DATA_HPP__

#include <any>
#include <string>
#include <vector>

#include "types.h"


namespace Ov {

    class Data {

    public:

        std::any data;

        Data(INT i) {
            data = i;
        }
        Data(float f) {
            data = static_cast<FLOAT>(f);
        }
        Data(double f) {
            data = static_cast<FLOAT>(f);
        }
        Data(char c) {
            data = c;
        }
        Data(bool b) {
            data = b;
        }
        Data(int i) : Data(static_cast<INT>(i)) {}
        template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
        Data(T i) : Data(static_cast<INT>(i)) {}

        Data(const char* str) {
            data = std::string(str);
        }
        Data(std::string const& str) {
            data = str;
        }

        template<typename T>
        Data(std::vector<T> const& vector) {
            std::vector<Data> array;

            for (auto const& value : vector)
                array.push_back(value);

            data = array;
        }

        template<typename T>
        Data(T const& t) {
            data = t;
        }


        operator INT() const {
            return std::any_cast<INT>(data);
        }
        operator float() const {
            return std::any_cast<FLOAT>(data);
        }
        operator double() const {
            return std::any_cast<FLOAT>(data);
        }
        operator char() const {
            return std::any_cast<char>(data);
        }
        operator bool() const {
            return std::any_cast<bool>(data);
        }
        template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
        operator T() const {
            return std::any_cast<T>(data);
        }

        operator const char* () const {
            return std::any_cast<std::string const&>(data).c_str();
        }
        operator std::string const& () const {
            return std::any_cast<std::string const&>(data);
        }

        template<typename T>
        operator std::vector<T>() {
            auto const& array = std::any_cast<std::vector<Data> const&>(data);
            std::vector<T> vector;

            for (auto const& data : array)
                vector.push_back(data);

            return vector;
        }

        template<typename T>
        operator T const& () {
            if (!data.has_value())
                data = T{};

            return std::any_cast<T const&>(data);
        }

        template<typename T, typename = std::enable_if_t<std::is_same<T, const char*>::value>>
        operator T* () {
            if (data.has_value())
                return std::any_cast<T*>(data);
            else
                return nullptr;
        }

    };

}


#endif
