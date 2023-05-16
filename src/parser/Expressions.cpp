#include "Expressions.hpp"


namespace Parser {

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
