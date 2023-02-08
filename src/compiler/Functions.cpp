#include "Functions.hpp"


namespace Analyzer {

    namespace Functions {

        inline auto & get_function(M<SymbolReference> reference) {
            return std::get<Object*>(*(*reference.begin()).get().begin())->functions;
        }

        auto separator_args = std::make_shared<Tuple>(
            std::make_shared<Symbol>("a"),
            std::make_shared<Symbol>("b")
        );
        M<Reference> separator(Context & context, bool potential) {
            return context["b"];
        }

        enum class Condition {
            TRUE,
            FALSE,
            UNDEFINED
        };
        Condition eval_condition(M<Data> conditions) {
            Condition r = Condition::UNDEFINED;
            for (auto condition : conditions) {
                if (auto c = std::get_if<bool>(&condition)) {
                    if (!condition.defined) {
                        r = Condition::UNDEFINED;
                        break;
                    }

                    if (r != Condition::FALSE && *c)
                        r = Condition::TRUE;
                    else if (r != Condition::TRUE && !*c)
                        r = Condition::FALSE;
                    else {
                        r = Condition::UNDEFINED;
                        break;
                    }
                } else throw FunctionArgumentsError();
            }
        }

        void if_function(M<Reference> & m, Context & context, bool potential, std::shared_ptr<Tuple> const& tuple, size_t i) {
            auto c = eval_condition(execute(context.get_parent(), potential, tuple->objects[i]).to_data(context));

            if (c != Condition::FALSE) {
                auto r = execute(context.get_parent(), potential || c == Condition::UNDEFINED, tuple->objects[i+1]);
                m.insert(r.begin(), r.end());
            }
            if (c != Condition::TRUE) {
                if (i+3 < tuple->objects.size()) {
                    auto else_s = execute(context.get_parent(), potential || c == Condition::UNDEFINED, tuple->objects[i+2]).to_data(context);
                    if (else_s == M<Reference>(context["else"]).to_data(context)) {
                        auto s = execute(context.get_parent(), potential || c == Condition::UNDEFINED, tuple->objects[i+3]).to_data(context);
                        if (i+4 == tuple->objects.size())
                            m.insert(s.begin(), s.end());
                        else if (i+6 < tuple->objects.size() && s == M<Reference>(context["if"]).to_data(context))
                            if_function(m, context, potential || c == Condition::UNDEFINED, tuple, i+4);
                        else throw FunctionArgumentsError();
                    } else throw FunctionArgumentsError();
                } else m.insert(M<Data>(context.new_object()));
            }
        }

        auto if_statement_args = std::make_shared<Symbol>("b");
        M<Reference> if_statement(Context & context, bool potential) {
            auto function = std::get<std::shared_ptr<FunctionDefinition>>(get_function(context["function"]).begin()->ptr)->body;
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(function)) {
                if (tuple->objects.size() >= 3) {
                    M<Reference> m;
                    if_function(m, context, potential, tuple, 0);
                    return m;
                } else throw FunctionArgumentsError();
            } else throw FunctionArgumentsError();
        }

        auto while_args = std::make_shared<Tuple>(
            std::make_shared<Symbol>("condition"),
            std::make_shared<Symbol>("block")
        );
        M<Reference> while_statement(Context & context, bool potential) {
            M<Reference> m;

            auto condition = get_function(context["condition"]);
            auto block = get_function(context["block"]);
            while (true) {
                auto c = eval_condition(call_function(context.get_parent(), potential, nullptr, condition, std::make_shared<Tuple>()).to_data(context));
                if (c == Condition::UNDEFINED)
                    potential = true;

                if (c != Condition::FALSE)
                    m = call_function(context.get_parent(), potential, nullptr, block, std::make_shared<Tuple>());
                else
                    break;
            }

            if (m.empty())
                m.insert(M<Data>(context.new_object()));
            return m;
        }

        auto for_statement_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("variable"),
            std::make_shared<Symbol>("from_s"),
            std::make_shared<Symbol>("begin"),
            std::make_shared<Symbol>("to_s"),
            std::make_shared<Symbol>("end"),
            std::make_shared<FunctionCall>(
                std::make_shared<Symbol>("block"),
                std::make_shared<Tuple>()
            )
        });
        M<Reference> for_statement(Context & context, bool potential) {
            try {
                auto & variable = context["variable"];
                auto & from_s = context["from_s"];
                auto begin = context["begin"].to_data();
                auto & to_s = context["to_s"];
                auto end = context["end"].to_data();
                auto & block = context["block"];

                bool defined = begin.size() * end.size() == 1;
                for (auto )

                if (from_s == context["from"] && to_s == context["to"]) {
                    for (auto begin_data : begin.to_data()) {
                        for (auto end_data : end.to_data()) {
                            for (long i = begin_data.get<long>(); i < end_data.get<long>(); i++) {
                                for ()
                                variable = i;
                                Interpreter::call_function(context.get_parent(), nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                            }
                        }
                    }

                    for (long i = begin; i < end; i++) {
                        variable = i;
                        Interpreter::call_function(context.get_parent(), nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                    }
                    return Reference(context.new_object());
                } else throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto for_step_statement_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
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
        });
        Reference for_step_statement(FunctionContext & context) {
            try {
                auto & parent = context.get_parent();

                auto variable = context["variable"];
                auto from_s = context["from_s"];
                auto begin = context["begin"].get<long>();
                auto to_s = context["to_s"];
                auto end = context["end"].get<long>();
                auto step_s = context["step_s"];
                auto step = context["step"].get<long>();
                auto block = context["block"];

                if (from_s == context["from"] && to_s == context["to"] && step_s == context["step"]) {
                    if (step > 0) {
                        for (long i = begin; i < end; i += step) {
                            variable = i;
                            Interpreter::call_function(parent, nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                        }
                    } else if (step < 0) {
                        for (long i = begin; i > end; i += step) {
                            variable = i;
                            Interpreter::call_function(parent, nullptr, std::get<Object*>(block)->functions, std::make_shared<Tuple>());
                        }
                    } else throw Interpreter::FunctionArgumentsError();
                    return Reference(context.new_object());
                } else throw Interpreter::FunctionArgumentsError();
            } catch (Data::BadAccess & e) {
                throw FunctionArgumentsError();
            }
        }

        auto try_statement_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<FunctionCall>(
                std::make_shared<Symbol>("try_block"),
                std::make_shared<Tuple>()
            ),
            std::make_shared<Symbol>("catch_s"),
            std::make_shared<Symbol>("catch_function"),
        });
        Reference try_statement(FunctionContext & context) {
            auto try_block = context["try_block"];
            auto catch_s = context["catch_s"];
            auto catch_function = context["catch_function"];

            if (catch_s == context["catch"]) {
                try {
                    return Interpreter::call_function(context.get_parent(), nullptr, try_block.get<Object*>()->functions, std::make_shared<Tuple>());
                } catch (Exception & ex) {
                    try {
                        return Interpreter::call_function(context.get_parent(), nullptr, catch_function.get<Object*>()->functions, ex.reference);
                    } catch (Interpreter::Error & e) {
                        throw ex;
                    }
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        auto throw_statement_args = std::make_shared<Symbol>("throw_expression");
        Reference throw_statement(FunctionContext & context) {
            Exception ex;
            ex.reference = context["throw_expression"];
            ex.position = context.position;
            ex.position->store_stack_trace(context.get_parent());
            throw ex;
        }

        auto path_args = std::make_shared<Symbol>("path");
        std::string get_canonical_path(FunctionContext & context) {
            try {
                auto path = context["path"].get<Object*>()->to_string();
                auto system_position = context.position->path;

                if (path[0] != '/')
                    path = system_position.substr(0, system_position.find_last_of("/")+1) + path;
                std::filesystem::path p(path);
                return std::filesystem::canonical(p).string();
            } catch (Data::BadAccess & e) {
                throw Interpreter::FunctionArgumentsError();
            } catch (std::exception & e) {
                throw Interpreter::FunctionArgumentsError();
            }
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

        auto copy_args = std::make_shared<Symbol>("data");
        Reference copy(FunctionContext & context) {
            auto data = context["data"];

            if (std::get_if<Object*>(&data))
                throw Interpreter::FunctionArgumentsError();
            else
                return Reference(Data(data));
        }

        Reference copy_pointer(FunctionContext & context) {
            return Reference(Data(context["data"]));
        }

        void assignation(M<Reference> const& var, M<Data> const& data, bool potential) {
            for (auto reference : var) {
                if (auto ref = std::get_if<SymbolReference>(&reference)) {
                    if (potential)
                        ref->get().insert(ref->get().end(), data.begin(), data.end());
                    else
                        ref->get() = data;
                } else if (auto tuple = std::get_if<TupleReference>(&reference)) {
                    for (auto d : data)
                        if (auto object = std::get_if<Object*>(&d)) {
                            if (tuple->size() == (*object)->array.size()) {
                                for (long i = 0; i < tuple->size(); i++)
                                    assignation((*tuple)[i], (*object)->array[i+1], potential);
                            } else throw FunctionArgumentsError();
                        } else throw FunctionArgumentsError();
                }
            }
        }

        auto assign_args = std::make_shared<Tuple>(
            std::make_shared<FunctionCall>(
                std::make_shared<Symbol>("var"),
                std::make_shared<Tuple>()
            ),
            std::make_shared<Symbol>("data")
        );
        M<Reference> assign(Context & context, bool potential) {
            auto var = call_function(context, potential, nullptr, get_function(context["var"]), std::make_shared<Tuple>());
            auto data = M<Reference>(context["data"]).to_data(context);

            assignation(var, data, potential);
            return var;
        }

        auto function_definition_args = std::make_shared<Tuple>(
            std::make_shared<Symbol>("var"),
            std::make_shared<Symbol>("data")
        );
        M<Reference> function_definition(Context & context, bool potential) {
            auto var = context["var"];
            auto function = M<Reference>(context["data"]).to_data(context);

            for (auto var_ref : var)
                for (auto var_data : var_ref.get()) if (auto var_object = std::get_if<Object*>(&var_data))
                    for (auto function_data : function) if (auto function_object = std::get_if<Object*>(&function_data))
                        for (auto it = (*function_object)->functions.rbegin(); it != (*function_object)->functions.rend(); it++)
                            (*var_object)->functions.push_front(*it);

            return var;
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

        auto equals_args = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
            std::make_shared<Symbol>("a"),
            std::make_shared<Symbol>("b")
        });
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

        M<Reference> not_check_pointers(FunctionContext & context, bool potential) {
            auto a = context["a"];
            auto b = context["b"];

            return Reference(Data(a != b));
        }

    }

}
