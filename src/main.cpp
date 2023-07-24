#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "interpreter/Interpreter.hpp"

#include "parser/Standard.hpp"


#ifdef __unix__
#include <unistd.h>
bool is_interactive() {
    return isatty(STDIN_FILENO);
}
#else
bool is_interactive() {
    return true;
}
#endif


int interactive_mode(std::string const& path) {
    Interpreter::GlobalContext context(nullptr);
    auto symbols = context.get_symbols();

    std::cout << "> ";

    std::string code;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.length() > 0) {
            code += line + '\n';

            try {
                auto expression = Parser::Standard(code, path).get_tree();
                context.expression = expression;

                auto new_symbols = expression->compute_symbols(symbols);
                symbols.insert(new_symbols.begin(), new_symbols.end());

                try {
                    auto r = Interpreter::execute(context, expression);

                    auto str = Interpreter::to_string(context, r);
                    if (!str.empty())
                        std::cout << str << std::endl;
                } catch (Interpreter::Exception const& ex) {
                    if (!ex.positions.empty()) {
                        ex.positions.front()->notify_error((std::ostringstream{} << "An exception occured: " << ex.reference.to_data(context)).str());
                        for (auto const& p : ex.positions)
                            p->notify_position();
                    }
                }

                std::cout << "> ";

                code = "";
            } catch (Parser::Standard::IncompleteCode const& e) {
                std::cout << '\t';
            } catch (Parser::Standard::Exception const& e) {
                std::cerr << e.what();
                std::cout << "> ";
                code = "";
            }
        } else {
            std::cout << "> ";
        }
    }

    std::cout << std::endl;
    return EXIT_SUCCESS;
}

int file_mode(std::string const& path, std::istream & is) {
    std::string code = (std::ostringstream{} << is.rdbuf()).str();

    try {
        auto expression = Parser::Standard(code, path).get_tree();

        Interpreter::GlobalContext context(expression);

        try {
            std::set<std::string> symbols = context.get_symbols();
            expression->compute_symbols(symbols);

            auto r = Interpreter::execute(context, expression);
            try {
                return static_cast<int>(r.to_data(context).get<long>());
            } catch (Interpreter::Data::BadAccess const& e) {
                return EXIT_SUCCESS;
            }
        } catch (Interpreter::Exception const& ex) {
            if (!ex.positions.empty()) {
                ex.positions.front()->notify_error((std::ostringstream{} << "An exception occured: " << ex.reference.to_data(context)).str());
                for (auto const& p : ex.positions)
                    p->notify_position();
            }
            return EXIT_FAILURE;
        }
    } catch (Parser::Standard::IncompleteCode & e) {
        std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
        return EXIT_FAILURE;
    } catch (Parser::Standard::Exception & e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }
}

int main(int argc, char ** argv) {
    if (argc == 1)
        if (is_interactive())
            return interactive_mode("stdin");
        else
            return file_mode("stdin", std::cin);
    else {
        std::ifstream src(argv[1]);
        if (src)
            return file_mode(argv[1], src);
        else {
            std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
            return EXIT_FAILURE;
        }
    }
}
