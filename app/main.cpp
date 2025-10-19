#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <boost/dll.hpp>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>
#include <ouverium/parser/Standard.hpp>

#include <ouverium/types.h>


std::filesystem::path const program_location = boost::filesystem::canonical(boost::dll::program_location()).parent_path().parent_path().string();
std::vector<std::string> include_path;

#ifdef READLINE
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
bool is_interactive() {
    return isatty(STDIN_FILENO) != 0;
}
bool get_line(std::string& line) {
    char* input = readline("> ");
    if (input != nullptr) {
        add_history(input);
        line = input;
        free(input);
    }
    return input != nullptr;
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
        if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if (f.get()) {
                if (line.length() > 0) {
                    ++line_number;
                    code += line + '\n';

                    try {
                        auto expression = Parser::Standard(code, "stdin").get_tree();
                        context->caller = expression;

                        auto new_symbols = expression->compute_symbols(symbols);
                        symbols.insert(new_symbols.begin(), new_symbols.end());

                        try {
                            auto r = Interpreter::execute(*context, expression);
                            try {
                                auto str = Interpreter::string_from(*context, r);
                                std::cout << str << std::endl;
                            } catch (Interpreter::Exception const&) {}
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
                return false;
            }
        }

        auto async = context->system.get<Interpreter::ObjectPtr>()->properties["async"];
        try {
            auto r = Interpreter::try_call_function(*context, nullptr, async, std::make_shared<Parser::Tuple>());
            if (auto* reference = std::get_if<Interpreter::Reference>(&r))
                return reference->to_data(*context).get<bool>();
        } catch (Interpreter::Exception const& ex) {
            ex.print_stack_trace(*context);
        }

        return true;
    }

    int on_exit() override {
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

    FileMode(std::string  path, std::istream& src) :
        path{ std::move(path) }, valid{ src } {
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
                std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"." << std::endl;
                error = true;
                return false;
            } catch (Parser::Standard::Exception const& e) {
                std::cerr << e.what();
                error = true;
                return false;
            }
        } else {
            std::cerr << "unable to load the source file \"" << path << "\"." << std::endl;
            return false;
        }
    }

    bool on_loop() override {
        auto async = context->system.get<Interpreter::ObjectPtr>()->properties["async"];
        try {
            auto r = Interpreter::try_call_function(*context, nullptr, async, std::make_shared<Parser::Tuple>());
            if (auto* reference = std::get_if<Interpreter::Reference>(&r))
                return reference->to_data(*context).get<bool>();
            else
                return false;
        } catch (Interpreter::Exception const& ex) {
            ex.print_stack_trace(*context);
            return false;
        }
    }

    int on_exit() override {
        if (error)
            return EXIT_FAILURE;
        else {
            try {
                return static_cast<int>(r.to_data(*context).get<OV_INT>());
            } catch (Interpreter::Exception const&) {
                return EXIT_SUCCESS;
            } catch (Interpreter::Data::BadAccess const&) {
                return EXIT_SUCCESS;
            }
        }
    }

};


#ifdef OUVERIUM_WXWIDGETS


#include <wx/wx.h>


class App : public wxApp {

public:

    std::unique_ptr<ExecutionMode> mode;

    bool OnInit() override {
        include_path.push_back((program_location / "libraries").string());

        if (argc == 1) {
            if (is_interactive())
                mode = std::make_unique<InteractiveMode>();
            else
                mode = std::make_unique<FileMode>("stdin", std::cin);
        } else if (argc == 2) {
            std::ifstream src{ std::string(argv[1]) };
            mode = std::make_unique<FileMode>(std::string(argv[1]), src);
        } else {
            std::cerr << "Usage: " << argv[0] << " [src]" << std::endl;
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

    include_path.push_back((program_location / "libraries").string());

    if (argc == 1) {
        if (is_interactive())
            mode = std::make_unique<InteractiveMode>();
        else
            mode = std::make_unique<FileMode>("stdin", std::cin);
    } else if (argc == 2) {
        std::ifstream src{ argv[1] };
        mode = std::make_unique<FileMode>(argv[1], src);
    } else {
        std::cerr << "Usage: " << argv[0] << " [src]" << std::endl;
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
