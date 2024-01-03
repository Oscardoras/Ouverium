#include <iostream>

#include <boost/dll.hpp>

#include "Dll.hpp"

#include "../../parser/Standard.hpp"


extern std::vector<std::string> include_path;

namespace Interpreter::SystemFunctions {

    namespace Dll {

        std::filesystem::path get_canonical_path(FunctionContext& context) {
            if (auto position = std::dynamic_pointer_cast<Parser::Standard::TextPosition>(context.expression->position)) {
                try {
                    auto path = std::filesystem::path(context["path"].to_data(context).get<Object*>()->to_string());

                    if (!path.is_absolute())
                        path = std::filesystem::path(position->path) / path;
                    return std::filesystem::canonical(path);
                } catch (std::exception const&) {
                    for (auto const& i : include_path) {
                        try {
                            auto path = std::filesystem::path(context["path"].to_data(context).get<Object*>()->to_string());

                            if (!path.is_absolute())
                                path = std::filesystem::path(i) / path;
                            return std::filesystem::canonical(path);
                        } catch (std::exception const&) {}
                    }

                    throw Interpreter::FunctionArgumentsError();
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        auto path_args = std::make_shared<Parser::Symbol>("path");

        Reference import(FunctionContext & context) {
            auto path = get_canonical_path(context);

            if (std::set<std::string>{".fl", ".ov", ".ouv"}.contains(path.extension())) {
                auto& global = context.get_global();
                auto root = context.expression->get_root();

                auto it = global.sources.find(path);
                if (it == global.sources.end()) {
                    std::ostringstream oss;
                    oss << std::ifstream(path).rdbuf();
                    std::string code = oss.str();

                    try {
                        auto expression = Parser::Standard(code, path).get_tree();
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
                        throw Exception(context, "incomplete code, you must finish the last expression in file \"" + std::string(path.c_str()) + "\"", context.get_global()["ParserException"].to_data(context), context.expression);
                    } catch (Parser::Standard::Exception const& e) {
                        throw Exception(context, e.what(), context.get_global()["ParserException"].to_data(context), context.expression);
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


        /*
                struct DllSymbol {
                    void* ptr;
                };

                auto getter_args = std::make_shared<Parser::Symbol>("var");
                Reference getter(FunctionContext& context) {
                    auto ref = context["var"];
                    if (auto property_reference = std::get_if<PropertyReference>(&ref)) {
                        try {
                            auto dll_symbol = std::any_cast<DllSymbol>(property_reference->parent.get().c_obj);


                        }
                        catch (std::bad_any_cast const&) {}
                    }

                    try {
                        auto dll_symbol = std::any_cast<DllSymbol>(ref.to_data(context).get<Object*>()->c_obj);


                    }
                    catch (Data::BadAccess const&) {}
                    catch (std::bad_any_cast const&) {}
                }

                static std::vector<boost::dll::shared_library> libraries;

                Reference import_dll(FunctionContext & context) {
                    auto path = get_canonical_path(context);

                    if (!path.has_extension()) {
                        auto& global = context.get_global();
                        auto root = context.expression->get_root();

                        for (auto const& l : libraries) {
                            if (std::filesystem::equivalent(l.location().c_str(), path))
                                return {};
                        }
                        libraries.push_back(std::move(boost::dll::shared_library(path.c_str(), boost::dll::load_mode::append_decorations)));
                        auto& library = libraries.back();

                        auto symbols = root->symbols;
                        for (auto const& symbol : context.expression->get_root()->get_symbols()) {
                            if (library.has(symbol)) {
                                symbols.insert(symbol);

                                try {
                                    global[symbol].to_data(global).get<Object*>()->c_obj = DllSymbol{ library.get<void*>(symbol) };
                                }
                                catch (Data::BadAccess const&) {
                                    // TODO
                                }
                            }
                        }
                        root->compute_symbols(symbols);

                    }
                    else throw Interpreter::FunctionArgumentsError();
                }
        */

        void init(GlobalContext& context) {
            context.get_function("import").push_front(SystemFunction{ path_args, import });
        }

    }

}
