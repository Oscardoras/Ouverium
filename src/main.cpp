#include <iostream>
#include <fstream>

#include "interpreter/Interpreter.hpp"

#include "parser/Standard.hpp"


int main(int argc, char ** argv) {
    if (argc == 1) {
        Interpreter::GlobalContext context;

        std::string code;
        std::string line;
        while (std::getline(std::cin, line))
            if (line.length() > 0) {
                code += line + '\n';
                try {
                    std::set<std::string> symbols;
                    for (auto const& symbol : context)
                        symbols.insert(symbol.first);

                    auto expression = Parser::Standard(code, ".").get_tree(symbols);
                    auto r = Interpreter::run(context, expression);
                    if (Interpreter::print(std::cout, r.to_data(context)))
                        std::cout << std::endl;
                    code = "";
                } catch (Parser::Standard::IncompleteCode const& e) {}
            }
    } else if (argc == 2) {
        std::ifstream src(argv[1]);
        if (src) {
            std::string code;
            std::string line;
            while (std::getline(src, line))
                code += line + '\n';

            Interpreter::GlobalContext context;
            try {
                std::set<std::string> symbols;
                for (auto const& symbol : context)
                    symbols.insert(symbol.first);

                auto expression = Parser::Standard(code, argv[1]).get_tree(symbols);
                auto r = Interpreter::run(context, expression);
            } catch (Parser::Standard::IncompleteCode & e) {
                std::cerr << "incomplete code, you must finish the last expression in file \"" << argv[1] << "\"" << std::endl;
            }
        } else std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
    } else std::cerr << "Error: please give the name of the source and the name of the destination." << std::endl;

	return 0;
}
