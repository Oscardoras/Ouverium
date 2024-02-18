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
            try {
                auto str = context["path"].to_data(context).get<Object*>()->to_string();

                if (auto position = std::dynamic_pointer_cast<Parser::Standard::TextPosition>(context.expression->position)) {
                    try {
                        auto path = std::filesystem::path(str);

                        if (!path.is_absolute())
                            path = std::filesystem::path(position->path) / path;
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

        auto path_args = std::make_shared<Parser::Symbol>("path");

        Reference import_system(FunctionContext& context) {
            try {
                if (context["path"].to_data(context).get<Object*>()->to_string() != "system")
                    throw Interpreter::FunctionArgumentsError();

                return Data(context.get_global().system);
            } catch (Data::BadAccess const&) {
                throw Interpreter::FunctionArgumentsError();
            }
        }


        // Import source file

        Reference import(FunctionContext & context) {
            auto path = get_canonical_path(context);

            if (std::set<std::string>{".fl", ".ov", ".ouv"}.contains(path.extension().string())) {
                auto& global = context.get_global();
                auto root = context.expression->get_root();

                auto it = global.sources.find(path);
                if (it == global.sources.end()) {
                    std::ostringstream oss;
                    oss << std::ifstream(path).rdbuf();
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
                        throw Exception(context, "incomplete code, you must finish the last expression in file \"" + path.string() + "\"", context.get_global()["ParserException"].to_data(context), context.expression);
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
                // Import c++ header

                struct CType {
                    std::string header_definition;

                    struct Property {
                        boost::dll::shared_library library;
                        CType& type;
                        std::function<Ov::Data()> getter;
                        std::function<void(Ov::Data)> setter;
                    };
                    struct Function {
                        enum class Type {
                            Bool, Int, Float, Char, String, Array, CObject
                        };
                        boost::dll::shared_library library;
                        CType& type;
                        std::function<Ov::Data(Ov::Data[])> function;
                    };
                    struct Array {
                        boost::dll::shared_library library;
                        CType& type;
                        std::function<Ov::Data()> getter;
                        std::function<void(Ov::Data)> setter;
                    };

                    std::map<std::string, Property> properties;
                    std::map<std::vector<Function::Type>, Function> functions;
                    std::unique_ptr<Array> array;
                };

                std::map<std::type_info*, CType> c_types;

                struct CSymbol {
                    boost::dll::shared_library library;
                    CType& type;

                    std::string symbol;
                    std::string include;
                };

                std::pair<CType::Function::Type, Ov::Data> get_data(Data const& data) {
                    if (auto c = get_if<char>(&data)) {
                        return { CType::Function::Type::Char , *c };
                    } else if (auto i = get_if<OV_INT>(&data)) {
                        return { CType::Function::Type::Int, *i };
                    } else if (auto f = get_if<OV_FLOAT>(&data)) {
                        return { CType::Function::Type::Float, *f };
                    } else if (auto b = get_if<bool>(&data)) {
                        return { CType::Function::Type::Bool, *b };
                    } else if (auto object = get_if<Object*>(&data)) {
                        if ((*object)->array.capacity() > 0) {
                            try {
                                return { CType::Function::Type::String, (*object)->to_string() };
                            } catch (Data::BadAccess const&) {
                                std::vector<Ov::Data> array;
                                for (auto const& d : (*object)->array)
                                    array.push_back(get_data(d).second);
                                return { CType::Function::Type::Array, array };
                            }
                        } else {
                            return { CType::Function::Type::CObject, static_cast<std::any&>((*object)->c_obj) };
                        }
                    } else return { CType::Function::Type::CObject, Data() };
                }

                Data get_data(Context& context, Ov::Data const& data) {
                    if (auto c = std::any_cast<char>(&data.data)) {
                        return *c;
                    } else if (auto i = std::any_cast<OV_INT>(&data.data)) {
                        return *i;
                    } else if (auto f = std::any_cast<OV_FLOAT>(&data.data)) {
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

                    auto args = std::get<CustomFunction>(context["function"].to_data(context).get<Object*>()->functions.front())->body;
                    auto& parent = context.get_parent();

                    std::vector<Data> data;
                    if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(args)) {
                        for (auto arg : tuple->objects)
                            data.push_back(Interpreter::execute(parent, arg).to_data(context));
                    } else {
                        data.push_back(Interpreter::execute(parent, args).to_data(context));
                    }

                    std::vector<CType::Function::Type> types;
                    std::vector<Ov::Data> arguments;
                    for (auto const& d : data) {
                        auto const& [type, ov_data] = get_data(d);
                        types.push_back(type);
                        arguments.push_back(ov_data);
                    }

                    if (auto c_symbol = std::any_cast<CSymbol>(&self->c_obj)) {

                        if (!c_symbol->type.functions.contains(types)) {
                            std::filesystem::create_directory("/tmp/ouverium_dll");
                            auto file = std::ofstream("/tmp/ouverium_dll/call.cpp");

                            std::string call = c_symbol->symbol + "(";
                            for (size_t i = 0; i < data.size();) {
                                call += "args[" + std::to_string(i) + "]";

                                ++i;
                                if (i < data.size())
                                    call += ", ";
                                else
                                    call += ")";
                            }

                            file << "#include <data.hpp>" << std::endl;
                            file << "#include \"" << c_symbol->include << "\"" << std::endl;
                            file << std::endl;

                            file << "namespace {" << std::endl;
                            file << "\ttemplate<typename T>" << std::endl;
                            file << "\tOv::Data Ov_" << c_symbol->symbol << "_caller_f(Ov::Data args[]) {" << std::endl;
                            file << "\t\treturn Ov::Data(" << call << ");" << std::endl;
                            file << "\t}" << std::endl;
                            file << "\ttemplate<>" << std::endl;
                            file << "\tOv::Data Ov_" << c_symbol->symbol << "_caller_f<void>(Ov::Data args[]) {" << std::endl;
                            file << "\t\t" << call << ";" << std::endl;
                            file << "\t\treturn Ov::Data{};" << std::endl;
                            file << "\t}" << std::endl;
                            file << "}" << std::endl;

                            file << std::endl;
                            file << "extern \"C\" Ov::Data Ov_" << c_symbol->symbol << "_caller(Ov::Data args[]) {" << std::endl;
                            file << "\treturn Ov_" << c_symbol->symbol << "_caller_f<decltype(" << call << ")>(args);" << std::endl;
                            file << "}" << std::endl;

                            file.close();

                            std::string cmd = "g++ -g --std=c++20 -shared -fPIC -o /tmp/ouverium_dll/" + c_symbol->symbol + ".so /tmp/ouverium_dll/call.cpp -I ";
                            cmd += (program_location / "capi_include").c_str();
                            if (system(cmd.c_str()) == 0) {
                                auto& caller = c_symbol->type.functions[types];
                                caller.library.load("/tmp/ouverium_dll/" + c_symbol->symbol + ".so");
                                auto f = caller.library.get<Ov::Data(Ov::Data[])>("Ov_" + c_symbol->symbol + "_caller");
                                caller.function = f;
                            } else throw FunctionArgumentsError();
                        }

                        auto caller = c_symbol->type.functions[types].function;
                        return get_data(context, caller(arguments.data()));
                    } else
                        return Data{};
                }

                auto getter_args = std::make_shared<Parser::Symbol>("var");
                Reference getter_c(FunctionContext& context) {
                    auto ref = context["var"];
                    if (ref.get_raw() == Data{})
                        throw FunctionArgumentsError();

                    if (auto property_reference = std::get_if<PropertyReference>(&ref)) {
                        try {
                            auto c_symbol = std::any_cast<CSymbol>(property_reference->parent.get().c_obj);

                            if (!c_symbol.type.properties.contains(property_reference->name)) {
                                std::filesystem::create_directory("/tmp/ouverium_dll");
                                auto file = std::ofstream("/tmp/ouverium_dll/call.cpp");

                                file << "#include <data.hpp>" << std::endl;
                                file << "#include \"" << c_symbol.include << "\"" << std::endl;
                                file << std::endl;

                                file << "extern \"C\" Ov::Data Ov_" << c_symbol.symbol << "_" << property_reference->name << "_getter(Ov::Data object) {" << std::endl;
                                file << "\treturn Ov::Data(object." << property_reference->name << ");" << std::endl;
                                file << "}" << std::endl;

                                file << "extern \"C\" void Ov_" << c_symbol.symbol << "_" << property_reference->name << "_setter(Ov::Data object, Ov::Data value) {" << std::endl;
                                file << "\tobject." << property_reference->name << " = value;" << std::endl;
                                file << "}" << std::endl;

                                file.close();

                                std::string cmd = "g++ -g --std=c++20 -shared -fPIC -o /tmp/ouverium_dll/" + c_symbol.symbol + ".so /tmp/ouverium_dll/call.cpp -I ";
                                cmd += (program_location / "capi_include").c_str();
                                if (system(cmd.c_str()) == 0) {
                                    auto& caller = c_symbol.callers[types];
                                    caller.library.load("/tmp/ouverium_dll/" + c_symbol.symbol + ".so");
                                    auto f = caller.library.get<Ov::Data(Ov::Data[])>("Ov_" + c_symbol.symbol + "_caller");
                                    caller.function = f;
                                } else throw FunctionArgumentsError();
                            }
                        } catch (std::bad_any_cast const&) {}

                        throw FunctionArgumentsError(); // TODO
                    } else {
                        try {
                            if (!ref.to_data(context).get<Object*>()->c_obj.has_value())
                                throw FunctionArgumentsError();
                        } catch (Data::BadAccess const&) {
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

                    bool std = path == path.stem();
                    if (std || std::set<std::string>{".h", ".hpp"}.contains(path.extension())) {
                        auto& global = context.get_global();
                        auto root = context.expression->get_root();

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
                            file << "#include <typeinfo>" << std::endl;
                            file << "#include " << path << std::endl;
                            file << "extern \"C\" std::type_info " << symbol << "_type = typeid(" << symbol << ");" << std::endl;
                            file.close();

                            std::string cmd = "g++ -shared -o /tmp/ouverium_dll/" + symbol + ".so /tmp/ouverium_dll/header.cpp 2> /dev/null";
                            if (system(cmd.c_str()) == 0) {
                                boost::dll::shared_library library("/tmp/ouverium_dll/" + symbol + ".so");
                                auto& type_info = library.get<std::type_info>(symbol + "_type");

                                try {
                                    auto object = global[symbol].to_data(context).get<Object*>();
                                    object->c_obj = CSymbol{ std::move(library), c_types[&type_info], symbol, path };

                                    symbols.insert(symbol);
                                } catch (Data::BadAccess const&) {}
                            }
                        }
                        root->compute_symbols(symbols);

                        return Reference();
                    } else throw Interpreter::FunctionArgumentsError();
                }
        */


        void init(GlobalContext& context) {
            context.get_function("import").push_front(SystemFunction{ path_args, import });
            //context.get_function("import").push_front(SystemFunction{ path_args, import_h });

            //context.get_function("getter").push_front(SystemFunction{ getter_args, getter_c });
            context.get_function("import").push_front(SystemFunction{ path_args, import_system });
        }

    }

}
