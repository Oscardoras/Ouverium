#include <algorithm>
#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "Base.hpp"

#include "../../parser/Standard.hpp"


namespace Interpreter {

    namespace Base {

        auto separator_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::Symbol>("b")
        }));
        Reference separator(FunctionContext & context) {
            return Reference(context["b"]);
        }

        auto if_statement_args = std::make_shared<Parser::FunctionCall>(
            std::make_shared<Parser::Symbol>("function"),
            std::make_shared<Parser::Tuple>()
        );
        Reference if_statement(FunctionContext & context) {
            try {
                auto function = std::get<CustomFunction>(static_cast<Data &>(context["function"]).get<Object*>(context)->functions.front())->body;

                if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(function)) {
                    if (tuple->objects.size() >= 2) {
                        auto & parent = context.get_parent();

                        if (Interpreter::execute(parent, tuple->objects[0]).to_data(context).get<bool>(context))
                            return Interpreter::execute(parent, tuple->objects[1]);
                        else {
                            unsigned long i = 2;
                            while (i < tuple->objects.size()) {
                                auto else_s = Interpreter::execute(parent, tuple->objects[i]).to_data(context);
                                if (else_s == context["else"] && i+1 < tuple->objects.size()) {
                                    auto s = Interpreter::execute(parent, tuple->objects[i+1]).to_data(context);
                                    if (s == context["if"] && i+3 < tuple->objects.size()) {
                                        if (Interpreter::execute(parent, tuple->objects[i+2]).to_data(context).get<bool>(context))
                                            return Interpreter::execute(parent, tuple->objects[i+3]);
                                        else i += 4;
                                    } else return s;
                                } else throw Interpreter::FunctionArgumentsError();
                            }
                            return Reference(Data(context.new_object()));
                        }
                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto while_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("condition"),
                std::make_shared<Parser::Tuple>()
            ),
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("block"),
                std::make_shared<Parser::Tuple>()
            )
        }));
        Reference while_statement(FunctionContext & context) {
            try {
                auto & parent = context.get_parent();
                Reference result;
                bool resulted = false;

                auto condition = static_cast<Data &>(context["condition"]).get<Object*>(context);
                auto block = static_cast<Data &>(context["block"]).get<Object*>(context);
                while (true) {
                    auto c = Interpreter::call_function(parent, nullptr, condition->functions, std::make_shared<Parser::Tuple>()).to_data(context).get<bool>(context);
                    if (c) {
                        result = Interpreter::call_function(parent, nullptr, block->functions, std::make_shared<Parser::Tuple>());
                        resulted = true;
                    } else break;
                }

                if (resulted)
                    return result;
                else
                    return Reference(Data(context.new_object()));
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto for_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("variable"),
            std::make_shared<Parser::Symbol>("from_s"),
            std::make_shared<Parser::Symbol>("begin"),
            std::make_shared<Parser::Symbol>("to_s"),
            std::make_shared<Parser::Symbol>("end"),
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("block"),
                std::make_shared<Parser::Tuple>()
            )
        }));
        Reference for_statement(FunctionContext & context) {
            try {
                auto & variable = static_cast<Data &>(context["variable"]);
                auto & from_s = static_cast<Data &>(context["from_s"]);
                auto & begin = static_cast<Data &>(context["begin"]).get<long>(context);
                auto & to_s = static_cast<Data &>(context["to_s"]);
                auto & end = static_cast<Data &>(context["end"]).get<long>(context);
                auto & block = static_cast<Data &>(context["block"]);

                if (from_s == context["from"] && to_s == context["to"]) {
                    for (long i = begin; i < end; i++) {
                        variable = i;
                        Interpreter::call_function(context.get_parent(), nullptr, block.get<Object*>(context)->functions, std::make_shared<Parser::Tuple>());
                    }
                    return Reference(context.new_object());
                } else throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto for_step_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("variable"),
            std::make_shared<Parser::Symbol>("from_s"),
            std::make_shared<Parser::Symbol>("begin"),
            std::make_shared<Parser::Symbol>("to_s"),
            std::make_shared<Parser::Symbol>("end"),
            std::make_shared<Parser::Symbol>("step_s"),
            std::make_shared<Parser::Symbol>("step"),
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("block"),
                std::make_shared<Parser::Tuple>()
            )
        }));
        Reference for_step_statement(FunctionContext & context) {
            try {
                auto & parent = context.get_parent();

                auto variable = context["variable"];
                auto from_s = context["from_s"];
                auto begin = static_cast<Data &>(context["begin"]).get<long>(context);
                auto to_s = context["to_s"];
                auto end = static_cast<Data &>(context["end"]).get<long>(context);
                auto step_s = context["step_s"];
                auto step = static_cast<Data &>(context["step"]).get<long>(context);
                auto block = context["block"];

                if (from_s == context["from"] && to_s == context["to"] && step_s == context["step"]) {
                    if (step > 0) {
                        for (long i = begin; i < end; i += step) {
                            static_cast<Data &>(variable) = i;
                            Interpreter::call_function(parent, nullptr, static_cast<Data &>(block).get<Object*>(context)->functions, std::make_shared<Parser::Tuple>());
                        }
                    } else if (step < 0) {
                        for (long i = begin; i > end; i += step) {
                            static_cast<Data &>(variable) = i;
                            Interpreter::call_function(parent, nullptr, static_cast<Data &>(block).get<Object*>(context)->functions, std::make_shared<Parser::Tuple>());
                        }
                    } else throw Interpreter::FunctionArgumentsError();
                    return Reference(context.new_object());
                } else throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto try_statement_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("try_block"),
                std::make_shared<Parser::Tuple>()
            ),
            std::make_shared<Parser::Symbol>("catch_s"),
            std::make_shared<Parser::Symbol>("catch_function")
        }));
        Reference try_statement(FunctionContext & context) {
            auto try_block = context["try_block"];
            auto catch_s = context["catch_s"];
            auto catch_function = context["catch_function"];

            if (catch_s == context["catch"]) {
                try {
                    return Interpreter::call_function(context.get_parent(), nullptr, static_cast<Data &>(try_block).get<Object*>(context)->functions, std::make_shared<Parser::Tuple>());
                } catch (Exception & ex) {
                    try {
                        return Interpreter::call_function(context.get_parent(), nullptr, static_cast<Data &>(catch_function).get<Object*>(context)->functions, ex.reference);
                    } catch (Interpreter::Error & e) {
                        throw ex;
                    }
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        auto throw_statement_args = std::make_shared<Parser::Symbol>("throw_expression");
        Reference throw_statement(FunctionContext & context) {
            Exception ex;
            ex.reference = context["throw_expression"];
            ex.position = context.get_position();
            ex.position->store_stack_trace(context.get_parent());
            throw ex;
        }

        auto path_args = std::make_shared<Parser::Symbol>("path");
        std::string get_canonical_path(FunctionContext & context) {
            if (auto position = std::dynamic_pointer_cast<Parser::Standard::TextPosition>(context.get_position())) {
                try {
                    auto path = static_cast<Data &>(context["path"]).get<Object*>(context)->to_string(context);
                    auto system_position = position->path;

                    if (path[0] != '/')
                        path = system_position.substr(0, system_position.find_last_of("/")+1) + path;
                    std::filesystem::path p(path);
                    return std::filesystem::canonical(p).string();
                } catch (Data::BadAccess & e) {
                    throw Interpreter::FunctionArgumentsError();
                } catch (std::exception & e) {
                    throw Interpreter::FunctionArgumentsError();
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference include(FunctionContext & context) {
            std::string path = get_canonical_path(context);

            auto & global = context.get_global();
            std::set<std::string> symbols;
            for (auto it : context)
                symbols.insert(it.first);

            std::ifstream file(path);
            std::string code;
            std::string line;
            while (std::getline(file, line))
                code += line + '\n';

            try {
                auto expression = Parser::Standard(code, path).get_tree(symbols);
                return Interpreter::execute(global, expression);
            } catch (Parser::Standard::IncompleteCode & e) {
                context.get_position()->store_stack_trace(context.get_parent());
                context.get_position()->notify_error("incomplete code, you must finish the last expression in file \"" + path + "\"");
                throw Error();
            }
        }

        Reference use(FunctionContext & context) {
            std::string path = get_canonical_path(context);

            auto & global = context.get_global();
            if (global.sources.find(path) == global.sources.end()) {
                std::set<std::string> symbols;
                for (auto it : global)
                    symbols.insert(it.first);

                std::ifstream file(path);
                std::string code;
                std::string line;
                while (std::getline(file, line))
                    code += line + '\n';

                try {
                    auto expression = Parser::Standard(code, path).get_tree(symbols);
                    global.sources[path] = expression;

                    for (auto const& symbol : expression->symbols)
                        global[symbol];

                    return Interpreter::execute(global, expression);
                } catch (Parser::Standard::IncompleteCode & e) {
                    context.get_position()->store_stack_trace(context.get_parent());
                    context.get_position()->notify_error("incomplete code, you must finish the last expression in file \"" + path + "\"");
                    throw Error();
                }
            } else
                return Reference(context.new_object());
        }

        auto copy_args = std::make_shared<Parser::Symbol>("data");
        Reference copy(FunctionContext & context) {
            auto data = context["data"];

            try {
                static_cast<Data &>(data).get<Object*>(context);
                throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess const& e) {
                return Reference(Data(data));
            }
        }

        Reference copy_pointer(FunctionContext & context) {
            return Reference(Data(context["data"]));
        }

        Reference assignation(Context & context, Reference var, Data d) {
            if (std::get_if<Data>(&var)) return d;
            else if (auto symbol_reference = std::get_if<SymbolReference>(&var)) symbol_reference->get() = d;
            else if (auto property_reference = std::get_if<PropertyReference>(&var)) static_cast<Data &>(*property_reference) = d;
            else if (auto array_reference = std::get_if<ArrayReference>(&var)) static_cast<Data &>(*array_reference) = d;
            else if (auto tuple_reference = std::get_if<TupleReference>(&var)) {
                try {
                    auto object = d.get<Object*>(context);
                    if (tuple_reference->size() == object->array.size()) {
                        for (size_t i = 0; i < tuple_reference->size(); i++)
                            assignation(context, (*tuple_reference)[i], object->array[i]);
                    } else throw Interpreter::FunctionArgumentsError();
                } catch (Data::BadAccess const& e) {
                    throw Interpreter::FunctionArgumentsError();
                }
            }
            return var;
        }

        auto assign_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("var"),
                std::make_shared<Parser::Tuple>()
            ),
            std::make_shared<Parser::Symbol>("data")
        }));
        Reference assign(FunctionContext & context) {
            auto var = Interpreter::call_function(context.get_parent(), nullptr, static_cast<Data &>(context["var"]).get<Object*>(context)->functions, std::make_shared<Parser::Tuple>());
            auto data = context["data"];

            return assignation(context, var, data);
        }

        auto function_definition_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("var"),
            std::make_shared<Parser::Symbol>("data")
        }));
        Reference function_definition(FunctionContext & context) {
            try {
                auto var = static_cast<Data &>(context["var"]).get<Object*>(context);
                auto object = static_cast<Data &>(context["data"]).get<Object*>(context);

                for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++)
                    var->functions.push_front(*it);

                return context["var"];
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto getter_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("var"),
            std::make_shared<Parser::FunctionCall>(
                std::make_shared<Parser::Symbol>("getter"),
                std::make_shared<Parser::Tuple>()
            )
        }));
        Reference getter(FunctionContext & context) {
            auto & var = static_cast<Data &>(context["var"]);
            auto getter = static_cast<Data &>(context["getter"]).get<Object*>(context);

            var = Getter(getter->functions.front());

            return var;
        }

        bool equals(RawData a, RawData b) {
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

        auto equals_args = std::make_shared<Parser::Tuple>(Parser::Tuple({
            std::make_shared<Parser::Symbol>("a"),
            std::make_shared<Parser::Symbol>("b")
        }));
        Reference equals(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

            return Reference(Data(equals(static_cast<Data &>(a).compute(context), static_cast<Data &>(b).compute(context))));
        }

        Reference not_equals(FunctionContext & context) {
            auto a = context["a"];
            auto b = context["b"];

            return Reference(Data(!equals(static_cast<Data &>(a).compute(context), static_cast<Data &>(b).compute(context))));
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

            Function if_s = SystemFunction{if_statement_args, if_statement};
            if_s.extern_symbols.emplace("if", context["if"]);
            if_s.extern_symbols.emplace("else", context["else"]);
            context.get_function("if").push_front(if_s);

            context.get_function("while").push_front(SystemFunction{while_statement_args, while_statement});

            Function for_s = SystemFunction{for_statement_args, for_statement};
            for_s.extern_symbols.emplace("from", context["from"]);
            for_s.extern_symbols.emplace("to", context["to"]);
            context.get_function("for").push_front(for_s);

            Function for_step_s = SystemFunction{for_step_statement_args, for_step_statement};
            for_step_s.extern_symbols.emplace("from", context["from"]);
            for_step_s.extern_symbols.emplace("to", context["to"]);
            for_step_s.extern_symbols.emplace("step", context["step"]);
            context.get_function("for").push_front(for_step_s);

            Function try_s = SystemFunction{try_statement_args, try_statement};
            try_s.extern_symbols.emplace("catch", context["catch"]);
            context.get_function("try").push_front(try_s);

            context.get_function("throw").push_front(SystemFunction{throw_statement_args, throw_statement});

            context.get_function("include").push_front(SystemFunction{path_args, include});
            context.get_function("using").push_front(SystemFunction{path_args, use});

            context.get_function("$").push_front(SystemFunction{copy_args, copy});
            context.get_function("$==").push_front(SystemFunction{copy_args, copy_pointer});

            context.get_function(":=").push_front(SystemFunction{assign_args, assign});
            context.get_function(":").push_front(SystemFunction{function_definition_args, function_definition});
            context.get_function("=").push_front(SystemFunction{getter_args, getter});

            context.get_function("==").push_front(SystemFunction{equals_args, equals});
            context.get_function("!=").push_front(SystemFunction{equals_args, not_equals});
            context.get_function("===").push_front(SystemFunction{equals_args, check_pointers});
            context.get_function("!==").push_front(SystemFunction{equals_args, not_check_pointers});
        }

    }

}
