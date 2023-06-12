#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>


namespace Interpreter {

    class Context;
    class Object;
    struct Function;

    class Getter {

    protected:

        Function* function;

    public:

        Getter(Function const& function);
        Getter(Getter const& getter);
        Getter(Getter && getter);

        ~Getter();

        Getter & operator=(Getter const& getter);
        Getter & operator=(Getter && getter);

        operator Function &();
        operator Function const&() const;

        friend bool operator==(Getter const&, Getter const&);

    };
    bool operator==(Getter const& a, Getter const& b);
    bool operator!=(Getter const& a, Getter const& b);

    struct Data : protected std::variant<Object*, char, double, long, bool, Getter> {

        class BadAccess: public std::exception {};

        using std::variant<Object*, char, double, long, bool, Getter>::variant;

        using std::variant<Object*, char, double, long, bool, Getter>::operator=;

        std::variant<Object*, char, double, long, bool, Getter> compute(Context & context) const;

        template<typename T>
        T & get(Context & context) {
            return const_cast<T &>(const_cast<Data const&>(*this).get<T>(context));
        }
        template<typename T>
        T const& get(Context & context) const {
            try {
                return std::get<T>(compute(context));
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }

        friend bool operator==(Data const&, Data const&);

    };

    bool operator==(Data const& a, Data const& b);
    bool operator!=(Data const& a, Data const& b);

}


#endif
