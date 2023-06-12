#include "Data.hpp"
#include "Function.hpp"


namespace Interpreter {

    Getter::Getter(Function const& function):
        function{ new Function(function) } {}
    Getter::Getter(Getter const& getter):
        function{ new Function(*getter.function) } {}
    Getter::Getter(Getter && getter):
        function{ getter.function } {
        getter.function = nullptr;
    }

    Getter::~Getter() {
        delete function;
    }

    Getter & Getter::operator=(Getter const& getter) {
        if (&getter != this) {
            delete function;

            function = new Function(*getter.function);
        }

        return *this;
    }
    Getter & Getter::operator=(Getter && getter) {
        if (&getter != this) {
            function = getter.function;

            getter.function = nullptr;
        }

        return *this;
    }

    Getter::operator Function &() {
        return *function;
    }
    Getter::operator Function const&() const {
        return *function;
    }

    bool operator==(Getter const& a, Getter const& b) {
        return a.function == b.function;
    }

    bool operator!=(Getter const& a, Getter const& b) {
        return !(a == b);
    }

}
