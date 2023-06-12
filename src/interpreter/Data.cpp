#include "Interpreter.hpp"


namespace Interpreter {

    std::variant<Object*, char, double, long, bool, Getter> Data::compute(Context & context) const {
        if (auto getter = std::get_if<Getter>(this))
            return call_function(context, nullptr, {*getter}, std::make_shared<Parser::Tuple>()).to_data(context);
        else
            return *this;
    }

    bool operator==(Data const& a, Data const& b) {
        return static_cast<std::variant<Object*, char, double, long, bool, Getter> const&>(a) == static_cast<std::variant<Object*, char, double, long, bool, Getter> const&>(b);
    }
    bool operator!=(Data const& a, Data const& b) {
        return !(a == b);
    }

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
