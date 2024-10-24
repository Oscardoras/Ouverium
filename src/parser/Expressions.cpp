#include "Expressions.hpp"

#include "../Types.hpp"


namespace Parser {

    std::shared_ptr<Expression> Expression::get_root() {
        auto root = shared_from_this();
        while (root->parent.lock() != nullptr)
            root = root->parent.lock();
        return root;
    }

    template<typename T>
    void merge(T const& from, T& to) {
        to.insert(from.begin(), from.end());
    }

    std::set<std::string> FunctionCall::get_symbols() const {
        std::set<std::string> symbols;

        merge(function->get_symbols(), symbols);
        merge(arguments->get_symbols(), symbols);

        return symbols;
    }

    std::set<std::string> FunctionCall::compute_symbols(std::set<std::string>& available_symbols) {
        function->parent = shared_from_this();
        arguments->parent = shared_from_this();

        merge(function->compute_symbols(available_symbols), symbols);
        merge(arguments->compute_symbols(available_symbols), symbols);

        return symbols;
    }

    std::set<std::string> FunctionDefinition::get_symbols() const {
        std::set<std::string> symbols;

        merge(parameters->get_symbols(), symbols);
        if (filter) merge(filter->get_symbols(), symbols);
        merge(body->get_symbols(), symbols);

        return symbols;
    }

    std::set<std::string> FunctionDefinition::compute_symbols(std::set<std::string>& available_symbols) {
        parameters->parent = shared_from_this();
        if (filter) filter->parent = shared_from_this();
        body->parent = shared_from_this();

        std::set<std::string> available_symbols_copy = available_symbols;

        merge(parameters->compute_symbols(available_symbols_copy), symbols);
        if (filter) merge(filter->compute_symbols(available_symbols_copy), symbols);
        merge(body->compute_symbols(available_symbols_copy), symbols);

        std::set<std::string> used_symbols;
        for (auto const& s : symbols)
            if (available_symbols.contains(s))
                used_symbols.insert(s);

        captures = used_symbols;

        return used_symbols;
    }

    std::set<std::string> Property::get_symbols() const {
        return object->get_symbols();
    }

    std::set<std::string> Property::compute_symbols(std::set<std::string>& available_symbols) {
        object->parent = shared_from_this();

        merge(object->compute_symbols(available_symbols), symbols);

        return symbols;
    }

    std::set<std::string> Symbol::get_symbols() const {
        if (std::holds_alternative<std::nullptr_t>(get_symbol(name)))
            return { name };
        else
            return {};
    }

    std::set<std::string> Symbol::compute_symbols(std::set<std::string>& available_symbols) {
        if (std::holds_alternative<std::nullptr_t>(get_symbol(name))) {
            symbols.insert(name);
            available_symbols.insert(name);
        }

        return symbols;
    }

    std::set<std::string> Tuple::get_symbols() const {
        std::set<std::string> symbols;

        for (auto const& ex : objects)
            merge(ex->get_symbols(), symbols);

        return symbols;
    }

    std::set<std::string> Tuple::compute_symbols(std::set<std::string>& available_symbols) {
        for (auto& ex : objects) {
            ex->parent = shared_from_this();
            merge(ex->compute_symbols(available_symbols), symbols);
        }

        return symbols;
    }

    std::string tabu(unsigned n) {
        std::string s;
        for (unsigned i = 0; i < n; ++i) s += "    ";
        return s;
    }

    std::string FunctionCall::to_string(unsigned n) const {
        std::string s;
        s += "FunctionCall:\n";
        n++;
        s += tabu(n) + "function: " + function->to_string(n);
        s += tabu(n) + "arguments: " + arguments->to_string(n);
        return s;
    }

    std::string FunctionDefinition::to_string(unsigned n) const {
        std::string s;
        s += "FunctionDefinition:\n";
        n++;
        s += tabu(n) + "parameters: " + parameters->to_string(n);
        if (filter != nullptr)
            s += tabu(n) + "filter: " + filter->to_string(n);
        s += tabu(n) + "body: " + body->to_string(n);
        return s;
    }

    std::string Property::to_string(unsigned int n) const {
        std::string s;
        s += "Property:\n";
        n++;
        if (object != nullptr)
            s += tabu(n) + "object: " + object->to_string(n);
        s += tabu(n) + "name: " + name + "\n";
        return s;
    }

    std::string Symbol::to_string(unsigned int /*n*/) const {
        return "Symbol: " + name + "\n";
    }

    std::string Tuple::to_string(unsigned int n) const {
        std::string s;
        s += "Tuple:\n";
        n++;
        for (auto const& ex : objects)
            s += tabu(n) + ex->to_string(n);
        return s;
    }

}
