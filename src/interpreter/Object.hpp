#ifndef __INTERPRETER_OBJECT_HPP__
#define __INTERPRETER_OBJECT_HPP__

#include <any>
#include <list>

#include "Function.hpp"


namespace Interpreter {

    struct Context;

    class CObj : public std::any {
    public:
        using std::any::any;

        template<typename T>
        T& get() {
            try {
                return std::any_cast<T&>(*this);
            } catch (std::bad_any_cast const&) {
                try {
                    return std::any_cast<std::reference_wrapper<T>>(*this).get();
                } catch (std::bad_any_cast const&) {
                    return *std::any_cast<std::shared_ptr<T>>(*this);
                }
            }
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

        IndirectReference operator[](std::string const& name);

        std::string to_string() const;

        void destruct(Context& context);

    };

    struct WeakReference {
        Object* obj;
    };

}


#endif
