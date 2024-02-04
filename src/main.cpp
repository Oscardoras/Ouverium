#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <boost/dll.hpp>

#include "compiler/SimpleAnalyzer.hpp"
#include "compiler/c/Translator.hpp"

#include "interpreter/Interpreter.hpp"

#include "parser/Standard.hpp"

#include "Types.hpp"


std::filesystem::path program_location = boost::filesystem::canonical(boost::dll::program_location()).parent_path().string();
std::vector<std::string> include_path;

int global_argc;
char** global_argv;

#ifdef READLINE
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
bool is_interactive() {
    return isatty(STDIN_FILENO);
}
bool get_line(std::string& line) {
    char* input = readline("> ");
    if (input) {
        add_history(input);
        line = input;
        free(input);
    }
    return input;
}
#else
bool is_interactive() {
    return true;
}
bool get_line(std::string& line) {
    std::cout << "> ";
    if (std::getline(std::cin, line))
        return true;
    else
        return false;
}
#endif


int interactive_mode(std::string const& path) {
    Interpreter::GlobalContext context(nullptr);
    auto symbols = context.get_symbols();

    size_t line_number = 0;
    std::string code;
    std::string line;
    while (get_line(line)) {
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
                    call_function(context, context.expression, context["print"], r);
                } catch (Interpreter::Exception const& ex) {
                    if (!ex.positions.empty()) {
                        std::ostringstream oss;
                        oss << "An exception occured: " << ex.reference.to_data(context);
                        ex.positions.front()->notify_error(oss.str());
                        for (auto const& p : ex.positions)
                            p->notify_position();
                    }
                }

                code = "";
                for (unsigned i = 0; i < line_number; ++i) code += '\n';
            } catch (Parser::Standard::IncompleteCode const&) {
                std::cout << '\t';
            } catch (Parser::Standard::Exception const& e) {
                std::cerr << e.what();
                code = "";
                for (unsigned i = 0; i < line_number; ++i) code += '\n';
            }
        }
    }

    return EXIT_SUCCESS;
}

int file_mode(std::string const& path, std::istream& is) {
    std::ostringstream oss;
    oss << is.rdbuf();
    std::string code = oss.str();

    try {
        auto expression = Parser::Standard(code, path).get_tree();

        Interpreter::GlobalContext context(expression);
        context.sources[std::filesystem::canonical(".")] = expression;

        try {
            std::set<std::string> symbols = context.get_symbols();
            expression->compute_symbols(symbols);

            auto r = Interpreter::execute(context, expression);
            try {
                return static_cast<int>(r.to_data(context).get<INT>());
            } catch (Interpreter::Data::BadAccess const&) {
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
    } catch (Parser::Standard::IncompleteCode const&) {
        std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
        return EXIT_FAILURE;
    } catch (Parser::Standard::Exception const& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }
}

void compile_mode(std::string const& path, std::istream& is, std::string const& out) {
    std::ostringstream oss;
    oss << is.rdbuf();
    std::string code = oss.str();

    auto expression = Parser::Standard(code, path).get_tree();
    std::set<std::string> symbols = Translator::CStandard::Translator::symbols;
    expression->compute_symbols(symbols);

    auto meta_data = Analyzer::simple_analize(expression);
    auto translator = Translator::CStandard::Translator(expression, meta_data);

    translator.translate(out);
    std::string cmd = "gcc -g -Wall -Wextra " + out + "/*.c -o " + out + "/executable -I " + (program_location / "capi_include").c_str() + " -Wl,-rpath," + program_location.c_str() + " build/libcapi.so";
    system(cmd.c_str());
}

int main(int argc, char** argv) {
    global_argc = argc;
    global_argv = argv;

    std::srand(std::time(nullptr));

    include_path.push_back(program_location.parent_path() / "libraries");

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
    } else if (argc == 3) {
        std::ifstream src(argv[1]);
        if (src) {
            compile_mode(argv[1], src, argv[2]);
            return EXIT_SUCCESS;
        } else {
            std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " [src] [out]" << std::endl;
        return EXIT_FAILURE;
    }
}
