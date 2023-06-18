#include "Expressions.hpp"


namespace Parser {

    std::shared_ptr<Expression> Expression::get_root() {
        auto root = shared_from_this();
        while (root->parent.lock() != nullptr)
            root = root->parent.lock();
        return root;
    }

    template<typename T>
    void merge(T const& from, T & to) {
        to.insert(from.begin(), from.end());
    }

    std::set<std::string> FunctionCall::compute_symbols(std::set<std::string> & available_symbols) {
        function->parent = shared_from_this();
        arguments->parent = shared_from_this();

        merge(function->compute_symbols(available_symbols), symbols);
        merge(arguments->compute_symbols(available_symbols), symbols);

        return symbols;
    }

    std::set<std::string> FunctionDefinition::compute_symbols(std::set<std::string> & available_symbols) {
        parameters->parent = shared_from_this();
        if (filter) filter->parent = shared_from_this();
        body->parent = shared_from_this();

        std::set<std::string> available_symbols_copy = available_symbols;

        merge(parameters->compute_symbols(available_symbols_copy), symbols);
        if (filter) merge(filter->compute_symbols(available_symbols_copy), symbols);
        merge(body->compute_symbols(available_symbols_copy), symbols);

        std::set<std::string> used_symbols;
        for (auto const& s : symbols)
            if (available_symbols.find(s) != available_symbols.end())
                used_symbols.insert(s);
        return used_symbols;
    }

    std::set<std::string> Property::compute_symbols(std::set<std::string> & available_symbols) {
        object->parent = shared_from_this();

        merge(object->compute_symbols(available_symbols), symbols);

        return symbols;
    }

    std::set<std::string> Symbol::compute_symbols(std::set<std::string> & available_symbols) {
        symbols.insert(name);
        available_symbols.insert(name);

        return symbols;
    }

    std::set<std::string> Tuple::compute_symbols(std::set<std::string> & available_symbols) {
        for (auto & ex : objects) {
            ex->parent = shared_from_this();
            merge(ex->compute_symbols(available_symbols), symbols);
        }

        return symbols;
    }

    std::string tabu(int n) {
        std::string s;
        for (int i = 0; i < n; i++) s+= "    ";
        return s;
    }

    std::string FunctionCall::to_string(unsigned int n) const {
        std::string s;
        s += "FunctionCall:\n";
        n++;
        s += tabu(n) + "function: " + function->to_string(n);
        s += tabu(n) + "arguments: " + arguments->to_string(n);
        return s;
    }

    std::string FunctionDefinition::to_string(unsigned int n) const {
        std::string s;
        s += "FunctionDefinition:\n";
        n++;
        s += tabu(n) + "parameters: " + parameters->to_string(n);
        s += tabu(n) + "filter: " + filter->to_string(n);
        s += tabu(n) + "body: " + body->to_string(n);
        return s;
    }

    std::string Property::to_string(unsigned int n) const {
        std::string s;
        s += "Property:\n";
        n++;
        s += tabu(n) + "object: " + object->to_string(n);
        s += tabu(n) + "name: " + name;
        return s;
    }

    std::string Symbol::to_string(unsigned int n) const {
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
