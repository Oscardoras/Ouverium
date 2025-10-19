#pragma once

// IWYU pragma: private; include "Interpreter.hpp"

#include <any>
#include <functional>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Data.hpp"
#include "Function.hpp"

namespace Interpreter {

    class CObj {

        std::any object;

    public:

        template<typename T>
        void set(std::reference_wrapper<T> t) {
            object = t;
        }

        template<typename T>
        void set(std::unique_ptr<T>&& t) {
            object = std::shared_ptr<T>(std::move(t));
        }

        [[nodiscard]] bool has_value() const {
            return object.has_value();
        }

        template<typename T>
        T& get() {
            if (auto* t = std::any_cast<std::reference_wrapper<T>>(&object))
                return t->get();
            else if (auto* t = std::any_cast<std::shared_ptr<T>>(&object))
                return **t;
            else
                throw Data::BadAccess();
        }
    };

    struct Object {

        std::map<std::string, Data> properties;
        std::list<Function> functions;
        std::vector<Data> array;
        CObj c_obj;

        Object() = default;
        Object(std::initializer_list<Data> const& array);
        Object(std::string const& str);

        [[nodiscard]] std::string to_string() const;

    };

}
