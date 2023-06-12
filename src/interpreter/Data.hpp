#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>


namespace Interpreter {

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

    struct Data : public std::variant<Object*, char, double, long, bool, Getter> {

        class BadAccess: public std::exception {};

        using std::variant<Object*, char, double, long, bool, Getter>::variant;

        template<typename T>
        T & get() {
            return const_cast<T &>(const_cast<Data const&>(*this).get<T>());
        }

        template<typename T>
        T const & get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }
    };

}


#endif
