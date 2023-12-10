#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "compiler/SimpleAnalyzer.hpp"
#include "compiler/c/Translator.hpp"

#include "interpreter/Interpreter.hpp"

#include "parser/Standard.hpp"


std::vector<std::string> include_path;

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

    unsigned long line_number = 0;
    std::string code;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.length() > 0) {
            ++line_number;
            code += line + '\n';

            try {
                auto expression = Parser::Standard(code, path).get_tree();
                context.expression = expression;

                auto new_symbols = expression->compute_symbols(symbols);
                symbols.insert(new_symbols.begin(), new_symbols.end());

                try {
                    auto r = Interpreter::execute(context, expression);

                    auto str = Interpreter::string_from(context, r);
                    if (!str.empty())
                        std::cout << str << std::endl;
                } catch (Interpreter::Exception const& ex) {
                    if (!ex.positions.empty()) {
                        std::ostringstream oss;
                        oss << "An exception occured: " << ex.reference.to_data(context);
                        ex.positions.front()->notify_error(oss.str());
                        for (auto const& p : ex.positions)
                            p->notify_position();
                    }
                }

                std::cout << "> ";
                code = "";
                for (unsigned long i = 0; i < line_number; ++i) code += '\n';
            } catch (Parser::Standard::IncompleteCode const& e) {
                std::cout << '\t';
            } catch (Parser::Standard::Exception const& e) {
                std::cerr << e.what();
                std::cout << "> ";
                code = "";
                for (unsigned long i = 0; i < line_number; ++i) code += '\n';
            }
        } else {
            std::cout << "> ";
        }
    }

    std::cout << std::endl;
    return EXIT_SUCCESS;
}

int file_mode(std::string const& path, std::istream & is) {
    std::ostringstream oss;
    oss << is.rdbuf();
    std::string code = oss.str();

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
                std::ostringstream oss;
                oss << "An exception occured: " << ex.reference.to_data(context);
                ex.positions.front()->notify_error(oss.str());
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

void compile_mode(std::string const& path, std::istream & is, std::string const& out) {
    std::ostringstream oss;
    oss << is.rdbuf();
    std::string code = oss.str();

    auto expression = Parser::Standard(code, path).get_tree();
    std::set<std::string> symbols = {";", ":", "print"};
    expression->compute_symbols(symbols);

    auto meta_data = Analyzer::simple_analize(expression);
    auto translator = Translator::CStandard::Translator(expression, meta_data);

    translator.translate(out);
    std::string cmd = "gcc " + out + "/*.c -o " + out + "/executable -I resources/library/include/ -Wl,-rpath,/home/oscar/Ouver/Ouverium libcapi.so";
    system(cmd.c_str());
}

int main(int argc, char ** argv) {
    auto p = std::filesystem::path(argv[0]).parent_path() / "libraries";
    include_path.push_back(p);

    auto p2 = std::filesystem::path(argv[0]).parent_path().parent_path() / "libraries";
    include_path.push_back(p2);

    if (argc == 1)
        if (is_interactive())
            return interactive_mode("stdin");
        else
            return file_mode("stdin", std::cin);
    else if (argc == 2) {
        std::ifstream src(argv[1]);
        if (src)
            return file_mode(argv[1], src);
        else {
            std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if (argc == 3) {
        std::ifstream src(argv[1]);
        if (src) {
            compile_mode(argv[1], src, argv[2]);
            return EXIT_SUCCESS;
        }
        else {
            std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
            return EXIT_FAILURE;
        }
    }
    else {
        std::cerr << "Usage: " << argv[0] << " [src] [out]" << std::endl;
        return EXIT_FAILURE;
    }
}
