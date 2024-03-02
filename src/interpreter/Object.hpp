#ifndef __INTERPRETER_OBJECT_HPP__
#define __INTERPRETER_OBJECT_HPP__

#include <any>
#include <list>

#include "Function.hpp"


namespace Interpreter {

    struct Context;

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

        bool has_value() const {
            return object.has_value();
        }

        template<typename T>
        T& get() {
            if (auto* t = std::any_cast<std::reference_wrapper<T>>(&object))
                return t->get();
            else if (auto* t = std::any_cast<std::shared_ptr<T>>(&object))
                return **t;
            else throw Data::BadAccess();
        }
    };

    struct Object {

        std::map<std::string, Data> properties;
        std::list<Function> functions;
        std::vector<Data> array;
        CObj c_obj;

        bool referenced = false;
        Object* weak_ref = nullptr;

        Object() = default;

        Object(std::string const& str) {
            array.reserve(str.size());
            for (auto c : str)
                array.push_back(Data(c));
        }

        std::string to_string() const;

        void destruct(Context& context);

    };

    struct WeakReference {
        Object* obj;
    };

}


#endif
