#include <iostream>
#include <fstream>

#include "interpreter/Interpreter.hpp"

#include "parser/Standard.hpp"


int main(int argc, char ** argv) {
    if (argc == 1) {
        Interpreter::GlobalContext context(nullptr);
        auto symbols = context.get_symbols();

        std::cout << "> ";

        std::string code;
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.length() > 0) {
                code += line + '\n';
                try {
                    auto expression = Parser::Standard(code, ".").get_tree();
                    context.expression = expression;

                    auto new_symbols = expression->compute_symbols(symbols);
                    symbols.insert(new_symbols.begin(), new_symbols.end());

                    auto r = Interpreter::run(context, expression);
                    if (Interpreter::print(context, std::cout, r.to_data(context)))
                        std::cout << std::endl;
                    code = "";

                    std::cout << "> ";
                } catch (Parser::Standard::IncompleteCode const& e) {}
            }
        }
        std::cout << std::endl;
    } else if (argc == 2) {
        std::ifstream src(argv[1]);
        if (src) {
            std::string code = Parser::Standard::read_file(src);

            try {

                auto expression = Parser::Standard(code, argv[1]).get_tree();

                Interpreter::GlobalContext context(expression);

                std::set<std::string> symbols = context.get_symbols();
                expression->compute_symbols(symbols);

                auto r = Interpreter::run(context, expression);
            } catch (Parser::Standard::IncompleteCode & e) {
                std::cerr << "incomplete code, you must finish the last expression in file \"" << argv[1] << "\"" << std::endl;
            }
        } else std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
    } else std::cerr << "Error: please give the name of the source and the name of the destination." << std::endl;

	return 0;
}
