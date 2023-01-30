#include <algorithm>
#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "Base.hpp"

#include "../../parser/Standard.hpp"


namespace Interpreter {

    namespace Base {

        auto separator_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("a"),
            std::make_shared<Symbol>("b")
        });
        Reference separator(FunctionContext & context) {
            return Reference(context["b"]);
        }

        Reference if_statement(FunctionContext & context) {
            auto function = std::get<CustomFunction>(*std::get<Object*>(context["function"])->functions.begin()).pointer->body;

            if (auto tuple = std::dynamic_pointer_cast<Tuple>(function)) {
                if (tuple->objects.size() >= 2) {
                    auto & parent = context.get_parent();

                    auto first_condition = Interpreter::execute(parent, tuple->objects[0]).to_data(context);
                    if (auto b = std::get_if<bool>(&first_condition)) {
                        if (*b)
                            return Interpreter::execute(parent, tuple->objects[1]);
                        else {
                            unsigned long i = 2;
                            while (i < tuple->objects.size()) {
                                auto else_s = Interpreter::execute(parent, tuple->objects[i]).to_data(context);
                                if (else_s == context["else"] && i+1 < tuple->objects.size()) {
                                    auto s = Interpreter::execute(parent, tuple->objects[i+1]).to_data(context);
                                    if (s == context["if"] && i+3 < tuple->objects.size()) {
                                        auto condition = Interpreter::execute(parent, tuple->objects[i+2]).to_data(context);
                                        if (b = std::get_if<bool>(&condition)) {
                                            if (*b)
                                                return Interpreter::execute(parent, tuple->objects[i+3]);
                                            else i += 4;
                                        } else throw Interpreter::FunctionArgumentsError();
                                    } else return s;
                                } else throw Interpreter::FunctionArgumentsError();
                            }
                            return Reference(context.new_object());
                        }
                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference while_statement(FunctionContext & context) {
            auto & parent = context.get_parent();
            Reference result;
            bool resulted = false;

            auto condition = context["condition"];
            auto block = context["block"];
            if (auto object_condition = std::get_if<Object*>(&condition)) {
                if (auto object_block = std::get_if<Object*>(&block)) {

                    while (true) {
                        auto c = Interpreter::call_function(parent, nullptr, (*object_condition)->functions, std::make_shared<Tuple>()).to_data(context);

                        if (auto b = std::get_if<bool>(&c)) {
                            if (b) {
                                result = Interpreter::call_function(parent, nullptr, (*object_block)->functions, std::make_shared<Tuple>());
                                resulted = true;
                            } else break;
                        } else throw Interpreter::FunctionArgumentsError();
                    }

                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();

            if (resulted) return result;
            else return Reference(context.new_object());
        }

        Reference for_statement(FunctionContext & context) {
            auto & variable = context["variable"];
            auto & from_s = context["from_s"];
            auto & begin = context["begin"];
            auto & to_s = context["to_s"];
            auto & end = context["end"];
            auto & block = context["block"];

            if (from_s == context["from"] && to_s == context["to"]) {
                if (auto begin_int = std::get_if<long>(&begin)) {
                    if (auto end_int = std::get_if<long>(&end)) {

                        for (long i = *begin_int; i < *end_int; i++) {
                            variable = i;
                            Interpreter::call_function(context.get_parent(), nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                        }
                        return Reference(context.new_object());

                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference for_step_statement(FunctionContext & context) {
            auto & parent = context.get_parent();

            auto variable = context["variable"];
            auto from_s = context["from_s"];
            auto begin = context["begin"];
            auto to_s = context["to_s"];
            auto end = context["end"];
            auto step_s = context["step_s"];
            auto step = context["step"];
            auto block = context["block"];

            if (from_s == context["from"] && to_s == context["to"] && step_s == context["step"]) {
                if (auto begin_int = std::get_if<long>(&begin)) {
                    if (auto end_int = std::get_if<long>(&end)) {
                        if (auto step_int = std::get_if<long>(&step)) {

                            if (*step_int > 0)
                                for (long i = *begin_int; i < *end_int; i += *step_int) {
                                    variable = i;
                                    Interpreter::call_function(parent, nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                                }
                            else if (*step_int < 0)
                                for (long i = *begin_int; i > *end_int; i += *step_int) {
                                    variable = i;
                                    Interpreter::call_function(parent, nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                                }
                            else throw Interpreter::FunctionArgumentsError();
                            return Reference(context.new_object());

                        } else throw Interpreter::FunctionArgumentsError();
                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference try_statement(FunctionContext & context) {
            auto try_block = context["try_block"];
            auto catch_s = context["catch_s"];
            auto catch_function = context["catch_function"];

            if (catch_s == context["catch"]) {
                try {
                    return Interpreter::call_function(context.get_parent(), nullptr, std::get<Object*>(try_block)->functions, std::make_shared<Tuple>());
                } catch (Exception & ex) {
                    try {
                        return Interpreter::call_function(context.get_parent(), nullptr, std::get<Object*>(catch_function)->functions, ex.reference);
                    } catch (Interpreter::Error & e) {
                        throw ex;
                    }
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference throw_statement(FunctionContext & context) {
            Exception ex;
            ex.reference = context["throw_expression"];
            ex.position = context.position;
            ex.position->store_stack_trace(context.get_parent());
            throw ex;
        }

        std::string get_canonical_path(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["path"])) {
                try {

                    auto path = (*object)->to_string();
                    auto system_position = context.position->path;

                    if (path[0] != '/')
                        path = system_position.substr(0, system_position.find_last_of("/")+1) + path;
                    std::filesystem::path p(path);
                    return std::filesystem::canonical(p).string();

                } catch (std::exception & e) {
                    throw Interpreter::FunctionArgumentsError();
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference include(FunctionContext & context) {
            std::string path = get_canonical_path(context);

            auto & global = context.get_global();
            std::vector<std::string> symbols;
            for (auto it = global.symbols.begin(); it != global.symbols.end(); it++)
                symbols.push_back(it->first);

            std::ifstream file(path);
            std::string code;
            std::string line;
            while (std::getline(file, line))
                code += line + '\n';

            try {
                auto expression = Parser::Standard::get_tree(code, path, symbols);
                return Interpreter::execute(global, expression);
            } catch (Parser::Standard::IncompleteCode & e) {
                context.position->store_stack_trace(context.get_parent());
                context.position->notify_error("incomplete code, you must finish the last expression in file \"" + path + "\"");
                throw Error();
            }
        }

        Reference use(FunctionContext & context) {
            std::string path = get_canonical_path(context);

            auto & global = context.get_global();
            if (global.files.find(path) == global.files.end()) {
                std::vector<std::string> symbols;
                for (auto it = global.symbols.begin(); it != global.symbols.end(); it++)
                    symbols.push_back(it->first);

                std::ifstream file(path);
                std::string code;
                std::string line;
                while (std::getline(file, line))
                    code += line + '\n';

                try {
                    auto expression = Parser::Standard::get_tree(code, path, symbols);
                    global.files[path] = expression;

                    for (auto const& symbol : expression->symbols)
                        global[symbol];

                    return Interpreter::execute(global, expression);
                } catch (Parser::Standard::IncompleteCode & e) {
                    context.position->store_stack_trace(context.get_parent());
                    context.position->notify_error("incomplete code, you must finish the last expression in file \"" + path + "\"");
                    throw Error();
                }
            } else
                return Reference(context.new_object());
        }

        Reference copy(FunctionContext & context) {
            auto data = context["data"];

            if (auto object = std::get_if<Object*>(&data))
                throw Interpreter::FunctionArgumentsError();
            else return data;
        }

        Reference copy_pointer(FunctionContext & context) {
            return Reference(context["data"]);
        }

        Reference assignation(Reference var, Data d) {
            if (auto data = std::get_if<Data>(&var)) return *data;
            else if (auto symbol_reference = std::get_if<SymbolReference>(&var)) symbol_reference->get() = d;
            else if (auto property_reference = std::get_if<PropertyReference>(&var)) property_reference->get() = d;
            else if (auto array_reference = std::get_if<ArrayReference>(&var)) array_reference->get() = d;
            else if (auto tuple_reference = std::get_if<TupleReference>(&var)) {
                if (auto object = std::get_if<Object*>(&d)) {
                    if (tuple_reference->size() == (*object)->array.size()) {
                        for (size_t i = 0; i < tuple_reference->size(); i++)
                            assignation((*tuple_reference)[i], (*object)->array[i]);
                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
            return var;
        }

        Reference assign(FunctionContext & context) {
            auto var = Interpreter::call_function(context, nullptr, std::get<Object*>(context["var"])->functions, std::make_shared<Tuple>());
            auto data = context["data"];

            return assignation(var, data);
        }

        Reference function_definition(FunctionContext & context) {
            try {
                auto var = std::get<Object*>(context["var"]);
                auto object = std::get<Object*>(context["data"]);

                for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++)
                    var->functions.push_front(*it);

                return context["var"];
            } catch (std::bad_variant_access & e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        bool equals(Data a, Data b) {
            if (auto a_object = std::get_if<Object*>(&a)) {
                if (auto b_object = std::get_if<Object*>(&b))
                    return (*a_object)->properties == (*b_object)->properties && (*a_object)->array == (*b_object)->array;
                else return false;
            } else if (auto a_char = std::get_if<char>(&a)) {
                if (auto b_char = std::get_if<char>(&b)) return *a_char == *b_char;
                else return false;
            } else if (auto a_float = std::get_if<double>(&a)) {
                if (auto b_float = std::get_if<double>(&b)) return *a_float == *b_float;
                else return false;
            } else if (auto a_int = std::get_if<long>(&a)) {
                if (auto b_int = std::get_if<long>(&b)) return *a_int == *b_int;
                else return false;
            } else if (auto a_bool = std::get_if<bool>(&a)) {
                if (auto b_bool = std::get_if<bool>(&b)) return *a_bool == *b_bool;
                else return false;
            } else throw FunctionArgumentsError();
        }

        Reference equals(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

            return Reference(Data(equals(a, b)));
        }

        Reference not_equals(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

            return Reference(Data(!equals(a, b)));
        }

        Reference check_pointers(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

            return Reference(Data(a == b));
        }

        Reference not_check_pointers(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];
            return Reference(Data(a != b));
        }

        void init(Context & context) {
            context.get_function(";").push_front(SystemFunction{separator_args, separator});

            Function if_s = SystemFunction{std::make_shared<FunctionCall>(
                std::make_shared<Symbol>("function"),
                std::make_shared<Tuple>()
            ), if_statement};
            if_s.extern_symbols["if"] = context["if"];
            if_s.extern_symbols["else"] = context["else"];
            context.get_function("if").push_front(if_s);

            context.get_function("while").push_front(SystemFunction{std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("condition"),
                    std::make_shared<Tuple>()
                ),
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("block"),
                    std::make_shared<Tuple>()
                )
            }), while_statement});

            Function for_s = SystemFunction{std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("variable"),
                std::make_shared<Symbol>("from_s"),
                std::make_shared<Symbol>("begin"),
                std::make_shared<Symbol>("to_s"),
                std::make_shared<Symbol>("end"),
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("block"),
                    std::make_shared<Tuple>()
                )
            }), for_statement};
            for_s.extern_symbols["from"] = context["from"];
            for_s.extern_symbols["to"] = context["to"];
            context.get_function("for").push_front(for_s);

            Function for_step_s = SystemFunction{std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("variable"),
                std::make_shared<Symbol>("from_s"),
                std::make_shared<Symbol>("begin"),
                std::make_shared<Symbol>("to_s"),
                std::make_shared<Symbol>("end"),
                std::make_shared<Symbol>("step_s"),
                std::make_shared<Symbol>("step"),
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("block"),
                    std::make_shared<Tuple>()
                )
            }), for_step_statement};
            for_step_s.extern_symbols["from"] = context["from"];
            for_step_s.extern_symbols["to"] = context["to"];
            for_step_s.extern_symbols["step"] = context["step"];
            context.get_function("for").push_front(for_step_s);

            Function try_s = SystemFunction{std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("try_block"),
                    std::make_shared<Tuple>()
                ),
                std::make_shared<Symbol>("catch_s"),
                std::make_shared<Symbol>("catch_function"),
            }), try_statement};
            try_s.extern_symbols["catch"] = context["catch"];
            context.get_function("try").push_front(try_s);

            auto equality = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("a"),
                std::make_shared<Symbol>("b")
            });

            context.get_function("throw").push_front(SystemFunction{std::make_shared<Symbol>("throw_expression"), throw_statement});
            context.get_function("include").push_front(SystemFunction{std::make_shared<Symbol>("path"), include});
            context.get_function("using").push_front(SystemFunction{std::make_shared<Symbol>("path"), use});
            context.get_function("$").push_front(SystemFunction{std::make_shared<Symbol>("object"), copy});
            context.get_function("$==").push_front(SystemFunction{std::make_shared<Symbol>("object"), copy_pointer});
            context.get_function(":=").push_front(SystemFunction{std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("var"),
                    std::make_shared<Tuple>()
                ),
                std::make_shared<Symbol>("object")
            }), assign});
            context.get_function(":").push_front(SystemFunction{std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("var"),
                std::make_shared<Symbol>("object")
            }), function_definition});
            context.get_function("==").push_front(SystemFunction{equality, (Reference (*)(FunctionContext & context)) equals});
            context.get_function("!=").push_front(SystemFunction{equality, not_equals});
            context.get_function("===").push_front(SystemFunction{equality, check_pointers});
            context.get_function("!==").push_front(SystemFunction{equality, not_check_pointers});
        }

    }

}
