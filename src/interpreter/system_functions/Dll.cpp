#include <fstream>

#include <boost/dll.hpp>

#include "SystemFunction.hpp"

#include "../../parser/Standard.hpp"


extern std::filesystem::path const program_location;
extern std::vector<std::string> include_path;

namespace Interpreter::SystemFunctions::Dll {

    std::filesystem::path get_canonical_path(FunctionContext& context) {
        try {
            auto str = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            auto position = context.caller ? context.caller->position.substr(0, context.caller->position.find(':')) : "";
            if (position.length() > 0) {
                try {
                    auto path = std::filesystem::path(str);

                    if (!path.is_absolute())
                        path = std::filesystem::path(position) / path;
                    return std::filesystem::canonical(path);
                } catch (std::exception const&) {
                    for (auto const& i : include_path) {
                        try {
                            auto path = std::filesystem::path(i) / str;

                            return std::filesystem::canonical(path);
                        } catch (std::exception const&) {
                            return str;
                        }
                    }

                    throw Interpreter::FunctionArgumentsError();
                }
            } else throw Interpreter::FunctionArgumentsError();
        } catch (Data::BadAccess const&) {
            throw Interpreter::FunctionArgumentsError();
        }
    }

    auto const path_args = std::make_shared<Parser::Symbol>("path");

    Reference import_system(FunctionContext& context) {
        try {
            if (context["path"].to_data(context).get<ObjectPtr>()->to_string() != "system")
                throw Interpreter::FunctionArgumentsError();

            return Data(context.get_global().system);
        } catch (Data::BadAccess const&) {
            throw Interpreter::FunctionArgumentsError();
        }
    }


    // Import source file

    Reference import(FunctionContext& context) {
        auto path = get_canonical_path(context);

        if (std::set<std::string>{".fl", ".ov", ".ouv"}.contains(path.extension().string())) {
            auto& global = context.get_global();
            auto root = context.caller->get_root();

            auto it = global.sources.find(path);
            if (it == global.sources.end()) {
                std::ostringstream oss;
                std::ifstream src(path);
                if (src) {
                    oss << src.rdbuf();
                    std::string code = oss.str();

                    try {
                        auto expression = Parser::Standard(code, path.string()).get_tree();
                        global.sources[path] = expression;

                        {
                            auto symbols = GlobalContext(nullptr).get_symbols();
                            expression->compute_symbols(symbols);
                        }

                        {
                            auto symbols = root->symbols;
                            symbols.insert(expression->symbols.begin(), expression->symbols.end());
                            root->compute_symbols(symbols);
                        }

                        return Interpreter::execute(global, expression);
                    } catch (Parser::Standard::IncompleteCode const&) {
                        throw Exception(context, context.caller, "incomplete code, you must finish the last expression in file \"" + path.string() + "\".");
                    } catch (Parser::Standard::Exception const& e) {
                        throw Exception(context, context.caller, e.what());
                    }
                } else {
                    throw Exception(context, context.caller, "Error: unable to load the source file \"" + path.string() + "\".");
                }
            } else {
                auto expression = it->second;

                auto symbols = root->symbols;
                symbols.insert(expression->symbols.begin(), expression->symbols.end());
                root->compute_symbols(symbols);

                return Reference();
            }
        } else throw Interpreter::FunctionArgumentsError();
    }


    void init(GlobalContext& context) {
        add_function(context["import"], path_args, import);

        //context.get_function("import").push_front(SystemFunction{ path_args, import_h });
        //context.get_function("getter").push_front(SystemFunction{ getter_args, getter_c });

        add_function(context["import"], path_args, import_system);
    }

}
