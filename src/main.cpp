#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include <boost/dll.hpp>

#include "compiler/SimpleAnalyzer.hpp"
#include "compiler/c/Translator.hpp"

#include "interpreter/Interpreter.hpp"

#include "parser/Standard.hpp"


std::filesystem::path program_location = boost::filesystem::canonical(boost::dll::program_location()).parent_path().string();
std::vector<std::string> include_path;

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


class ExecutionMode {
public:
    virtual bool on_init() = 0;
    virtual bool on_loop() = 0;
    virtual int on_exit() = 0;
    virtual ~ExecutionMode() = default;
};

class InteractiveMode : public ExecutionMode {

    std::unique_ptr<Interpreter::GlobalContext> context;
    std::set<std::string> symbols;

    size_t line_number = 0;
    std::string code;
    std::string line;

    std::function<bool()> async_read;
    std::future<bool> f;

public:

    bool on_init() override {
        context = std::make_unique<Interpreter::GlobalContext>(nullptr);
        symbols = context->get_symbols();

        async_read = [this]() {
            return get_line(line);
        };
        f = std::async(std::launch::async, async_read);

        return true;
    }

    bool on_loop() override {
        context->ioc.poll();

        if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if (f.get()) {
                if (line.length() > 0) {
                    ++line_number;
                    code += line + '\n';

                    try {
                        auto expression = Parser::Standard(code, "stdin").get_tree();
                        context->expression = expression;

                        auto new_symbols = expression->compute_symbols(symbols);
                        symbols.insert(new_symbols.begin(), new_symbols.end());

                        try {
                            auto r = Interpreter::execute(*context, expression);
                            auto str = Interpreter::string_from(*context, r);
                            if (!str.empty() && str != "()")
                                std::cout << str << std::endl;
                        } catch (Interpreter::Exception const& ex) {
                            ex.print_stack_trace(*context);
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

                f = std::async(std::launch::async, async_read);
            } else {
                f = std::async(std::launch::async, []() {
                    return false;
                });
                return !context->ioc.stopped();
            }
        }

        return true;
    }

    int on_exit() override {
        context = nullptr;

        return EXIT_SUCCESS;
    }

};

class FileMode : public ExecutionMode {

    std::string path;
    bool valid;
    std::string code;

    std::unique_ptr<Interpreter::GlobalContext> context;

    Interpreter::Reference r;
    bool error = false;

public:

    FileMode(std::string const& path, std::istream& src) :
        path{ path }, valid{ src } {
        if (valid) {
            std::ostringstream oss;
            oss << src.rdbuf();
            code = oss.str();
        }
    }

    bool on_init() override {
        if (valid) {
            try {
                auto expression = Parser::Standard(code, path).get_tree();

                context = std::make_unique<Interpreter::GlobalContext>(expression);
                context->sources[std::filesystem::canonical(".")] = expression;

                try {
                    std::set<std::string> symbols = context->get_symbols();
                    expression->compute_symbols(symbols);

                    r = Interpreter::execute(*context, expression);
                    return true;
                } catch (Interpreter::Exception const& ex) {
                    ex.print_stack_trace(*context);
                    error = true;
                    return false;
                }
            } catch (Parser::Standard::IncompleteCode const&) {
                std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
                error = true;
                return false;
            } catch (Parser::Standard::Exception const& e) {
                std::cerr << e.what();
                error = true;
                return false;
            }
        } else {
            std::cerr << "Error: unable to load the source file " << path << "." << std::endl;
            return false;
        }
    }

    bool on_loop() override {
        context->ioc.poll();

        return !context->ioc.stopped();
    }

    int on_exit() override {
        auto ptr = std::move(context);

        if (error)
            return EXIT_FAILURE;
        else {
            try {
                return static_cast<int>(r.to_data(*context).get<OV_INT>());
            } catch (Interpreter::Data::BadAccess const&) {
                return EXIT_SUCCESS;
            }
        }
    }

};

class CompileMode : public ExecutionMode {

    std::string path;
    std::ifstream src;
    std::string out;

public:

    CompileMode(std::string const& path, std::string const& out) :
        path{ path }, src{ path }, out{ out } {}

    bool on_init() override {
        if (src) {
            std::ostringstream oss;
            oss << src.rdbuf();
            std::string code = oss.str();

            auto expression = Parser::Standard(code, path).get_tree();
            std::set<std::string> symbols = Translator::CStandard::Translator::symbols;
            expression->compute_symbols(symbols);

            auto meta_data = Analyzer::simple_analize(expression);
            auto translator = Translator::CStandard::Translator(expression, meta_data);

            translator.translate(out);
            std::string cmd = "gcc -g -Wall -Wextra " + out + "/*.c -o " + out + "/executable -I " + (program_location / "include").string() + " -Wl,-rpath," + program_location.string() + " build/libcapi.so";
            system(cmd.c_str());
        } else {
            std::cerr << "Error: unable to load the source file " << path << "." << std::endl;
        }

        return false;
    }

    bool on_loop() override {
        return false;
    }

    int on_exit() override {
        return src ? EXIT_SUCCESS : EXIT_FAILURE;
    }

};



#ifdef WXWIDGETS


#include <wx/wx.h>


class App : public wxApp {

public:

    std::unique_ptr<ExecutionMode> mode;

    bool OnInit() override {
        std::srand(std::time(nullptr));
        include_path.push_back((program_location.parent_path() / "libraries").string());

        if (argc == 1) {
            if (is_interactive())
                mode = std::make_unique<InteractiveMode>();
            else
                mode = std::make_unique<FileMode>("stdin", std::cin);
        } else if (argc == 2) {
            std::ifstream src{ std::string(argv[1]) };
            mode = std::make_unique<FileMode>(std::string(argv[1]), src);
        } else if (argc == 3) {
            mode = std::make_unique<CompileMode>(std::string(argv[1]), std::string(argv[2]));
        } else {
            std::cerr << "Usage: " << argv[0] << " [src] [out]" << std::endl;
            return false;
        }

        if (mode->on_init()) {
            Bind(wxEVT_IDLE, [this](wxIdleEvent& e) {
                if (mode->on_loop())
                    e.RequestMore();
                else if (!wxTheApp->GetTopWindow())
                    wxTheApp->ExitMainLoop();
            });
            return true;
        } else
            return false;
    }

    int OnExit() override {
        if (mode)
            return mode->on_exit();
        else
            return EXIT_FAILURE;
    }
};

wxIMPLEMENT_APP(App);


#else


int main(int argc, char** argv) {
    std::unique_ptr<ExecutionMode> mode;

    std::srand(std::time(nullptr));
    include_path.push_back((program_location.parent_path() / "libraries").string());

    if (argc == 1) {
        if (is_interactive())
            mode = std::make_unique<InteractiveMode>();
        else
            mode = std::make_unique<FileMode>("stdin", std::cin);
    } else if (argc == 2) {
        std::ifstream src{ argv[1] };
        mode = std::make_unique<FileMode>(argv[1], src);
    } else if (argc == 3) {
        mode = std::make_unique<CompileMode>(argv[1], argv[2]);
    } else {
        std::cerr << "Usage: " << argv[0] << " [src] [out]" << std::endl;
        return EXIT_FAILURE;
    }

    if (mode->on_init()) {
        while (mode->on_loop());
    }

    if (mode)
        return mode->on_exit();
    else
        return EXIT_FAILURE;
}


#endif
