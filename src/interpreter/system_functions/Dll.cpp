#include <functional>
#include <fstream>

#include <boost/dll.hpp>

#include <data.hpp>

#include "Dll.hpp"

#include "../../parser/Standard.hpp"


extern std::filesystem::path program_location;
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


        struct CSymbol {
            enum class Type {
                Bool, Int, Float, Char, String, Array, CObject
            };
            struct Caller {
                boost::dll::shared_library library;
                std::function<Ov::Data(Ov::Data[])> function;
            };
            std::string symbol;
            std::string include;
            std::map<std::vector<CSymbol::Type>, Caller> callers;
        };

        std::pair<CSymbol::Type, Ov::Data> get_data(Data const& data) {
            if (auto c = get_if<char>(&data)) {
                return { CSymbol::Type::Char , *c };
            } else if (auto i = get_if<INT>(&data)) {
                return { CSymbol::Type::Int, *i };
            } else if (auto f = get_if<FLOAT>(&data)) {
                return { CSymbol::Type::Float, *f };
            } else if (auto b = get_if<bool>(&data)) {
                return { CSymbol::Type::Bool, *b };
            } else if (auto object = get_if<Object*>(&data)) {
                if ((*object)->array.capacity() > 0) {
                    try {
                        return { CSymbol::Type::String, (*object)->to_string() };
                    } catch (Data::BadAccess const&) {
                        std::vector<Ov::Data> array;
                        for (auto const& d : (*object)->array)
                            array.push_back(get_data(d).second);
                        return { CSymbol::Type::Array, array };
                    }
                } else {
                    return { CSymbol::Type::CObject, static_cast<std::any&>((*object)->c_obj) };
                }
            } else return { CSymbol::Type::CObject, Data(nullptr) };
        }

        Data get_data(Context& context, Ov::Data const& data) {
            if (auto c = std::any_cast<char>(&data.data)) {
                return *c;
            } else if (auto i = std::any_cast<INT>(&data.data)) {
                return *i;
            } else if (auto f = std::any_cast<FLOAT>(&data.data)) {
                return *f;
            } else if (auto b = std::any_cast<bool>(&data.data)) {
                return *b;
            } else if (auto str = std::any_cast<std::string>(&data.data)) {
                return context.new_object(*str);
            } else if (auto vector = std::any_cast<std::vector<Ov::Data>>(&data.data)) {
                auto object = context.new_object();
                for (auto const& d : *vector)
                    object->array.push_back(get_data(context, d));
                return object;
            } else {
                auto object = context.new_object();
                object->c_obj = data.data;
                return object;
            }
        }

        auto c_function_args = std::make_shared<Parser::FunctionCall>(
            std::make_shared<Parser::Symbol>("function"),
            std::make_shared<Parser::Tuple>()
        );
        Reference c_function(FunctionContext& context) {
            auto self = context["this"].to_data(context).get<Object*>();
            auto& c_symbol = std::any_cast<CSymbol&>(self->c_obj);
            auto args = std::get<CustomFunction>(context["function"].to_data(context).get<Object*>()->functions.front())->body;
            auto& parent = context.get_parent();

            std::vector<Data> data;

            if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(args)) {
                for (auto arg : tuple->objects)
                    data.push_back(Interpreter::execute(parent, arg).to_data(context));
            } else {
                data.push_back(Interpreter::execute(parent, args).to_data(context));
            }

            std::vector<CSymbol::Type> types;
            std::vector<Ov::Data> arguments;
            for (auto const& d : data) {
                auto const& [type, ov_data] = get_data(d);
                types.push_back(type);
                arguments.push_back(ov_data);
            }

            if (!c_symbol.callers.contains(types)) {
                std::filesystem::create_directory("/tmp/ouverium_dll");
                auto file = std::ofstream("/tmp/ouverium_dll/call.cpp");

                file << "#include <data.hpp>" << std::endl;
                file << "#include \"" << c_symbol.include << "\"" << std::endl;
                file << std::endl;
                file << "extern \"C\" Ov::Data Ov_" << c_symbol.symbol << "_caller(Ov::Data args[]) {" << std::endl;
                file << "\treturn Ov::Data(" << c_symbol.symbol << "(";
                for (size_t i = 0; i < data.size();) {
                    file << "args[" << i << "]";

                    ++i;
                    if (i < data.size())
                        file << ", ";
                }
                file << "));" << std::endl;
                file << "}" << std::endl;

                file.close();

                std::string cmd = "g++ -g -shared -fPIC -o /tmp/ouverium_dll/" + c_symbol.symbol + ".so /tmp/ouverium_dll/call.cpp -I ";
                cmd += (program_location / "capi_include").c_str();
                cmd += " " + c_symbol.include;
                if (system(cmd.c_str()) == 0) {
                    auto& caller = c_symbol.callers[types];
                    caller.library.load("/tmp/ouverium_dll/" + c_symbol.symbol + ".so");
                    auto f = caller.library.get<Ov::Data(Ov::Data[])>("Ov_" + c_symbol.symbol + "_caller");
                    caller.function = f;
                } else throw FunctionArgumentsError();
            }

            auto caller = c_symbol.callers[types].function;
            return get_data(context, caller(arguments.data()));
        }

        auto getter_args = std::make_shared<Parser::Symbol>("var");
        Reference getter_c(FunctionContext& context) {
            auto ref = context["var"];
            if (ref.get_raw() == Data{})
                throw FunctionArgumentsError();

            if (auto property_reference = std::get_if<PropertyReference>(&ref)) {
                try {
                    auto c_symbol = std::any_cast<CSymbol>(property_reference->parent.get().c_obj);

                } catch (std::bad_any_cast const&) {}

                throw FunctionArgumentsError(); // TODO
            } else {
                try {
                    std::any_cast<CSymbol>(ref.to_data(context).get<Object*>()->c_obj);
                } catch (Data::BadAccess const&) {
                    throw FunctionArgumentsError();
                } catch (std::bad_any_cast const&) {
                    throw FunctionArgumentsError();
                }

                Function function = SystemFunction{ c_function_args, c_function };
                function.extern_symbols.emplace("this", ref);

                auto object = context.new_object();
                object->functions.push_front(function);
                return Data(object);
            }
        }

        Reference import_h(FunctionContext& context) {
            auto path = get_canonical_path(context);

            if (std::set<std::string>{".h", ".hpp"}.contains(path.extension())) {
                auto& global = context.get_global();
                auto root = context.expression->get_root();

                auto it = global.sources.find(path);
                if (it == global.sources.end()) {
                    global.sources[path] = std::make_shared<Parser::Tuple>();

                    auto symbols = root->symbols;
                    for (auto const& symbol : context.expression->get_root()->get_symbols()) {
                        bool c_symbol = true;
                        for (char c : symbol) {
                            if (!(std::isalnum(c) || c == '_')) {
                                c_symbol = false;
                                break;
                            }
                        }
                        if (!c_symbol)
                            continue;

                        std::filesystem::create_directory("/tmp/ouverium_dll");
                        auto file = std::ofstream("/tmp/ouverium_dll/header.cpp");
                        file << "#include " << path << std::endl;
                        file << "void* ptr = (void*) &" << symbol << ";" << std::endl;
                        file.close();

                        if (system("g++ -shared -o /tmp/ouverium_dll/libheader.so /tmp/ouverium_dll/header.cpp 2> /dev/null") == 0) {
                            try {
                                auto object = global[symbol].to_data(context).get<Object*>();
                                object->c_obj = CSymbol{ symbol, path, {} };

                                symbols.insert(symbol);
                            } catch (Data::BadAccess const&) {}
                        }
                    }
                    root->compute_symbols(symbols);
                }

                return Reference();
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
            context.get_function("import").push_front(SystemFunction{ path_args, import_h });

            context.get_function("getter").push_front(SystemFunction{ getter_args, getter_c });
        }

    }

}
