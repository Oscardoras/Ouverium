#include <algorithm>
#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "Base.hpp"
#include "../../parser/Standard.hpp"


namespace Interpreter {

    namespace Base {

        Reference separator(FunctionContext & context) {
            return Reference(context.get_symbol("b"));
        }

        Reference if_statement(FunctionContext & context) {
            auto function = ((CustomFunction&) **context.get_symbol("function").to_object(context)->functions.begin()).pointer->body;

            if (auto tuple = std::dynamic_pointer_cast<Tuple>(function)) {
                if (tuple->objects.size() >= 2) {
                    auto & parent = context.get_parent();

                    auto first_condition = Interpreter::execute(parent, tuple->objects[0]).to_object(context);
                    if (first_condition->type == Object::Bool) {
                        if (first_condition->data.b)
                            return Interpreter::execute(parent, tuple->objects[1]).to_object(context);
                        else {
                            unsigned long i = 2;
                            while (i < tuple->objects.size()) {
                                auto else_s = Interpreter::execute(parent, tuple->objects[i]).to_object(context);
                                if (else_s == context.get_symbol("else").to_object(context) && i+1 < tuple->objects.size()) {
                                    auto s = Interpreter::execute(parent, tuple->objects[i+1]).to_object(context);
                                    if (s == context.get_symbol("if").to_object(context)) {
                                        if (i+3 < tuple->objects.size()) {
                                            auto condition = Interpreter::execute(parent, tuple->objects[i+2]).to_object(context);
                                            if (condition->type == Object::Bool) {
                                                if (condition->data.b)
                                                    return Interpreter::execute(parent, tuple->objects[i+3]).to_object(context);
                                                else i += 4;
                                            } else throw Interpreter::FunctionArgumentsError();
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

            auto condition = context.get_symbol("condition").to_object(context);
            auto block = context.get_symbol("block").to_object(context);
            while (true) {
                auto c = Interpreter::call_function(parent, nullptr, condition->functions, std::make_shared<Tuple>()).to_object(context);

                if (c->type == Object::Bool) {
                    if (c->data.b) {
                        result = Interpreter::call_function(parent, nullptr, block->functions, std::make_shared<Tuple>());
                    } else break;
                } else throw Interpreter::FunctionArgumentsError();
            }

            if (result.type == Reference::Pointer && result.pointer == nullptr)
                return Reference(context.new_object());
            else return result;
        }

        Reference for_statement(FunctionContext & context) {
            auto & parent = context.get_parent();

            auto variable = context.get_symbol("variable");
            auto from_s = context.get_symbol("from_s").to_object(context);
            auto begin = context.get_symbol("begin").to_object(context);
            auto to_s = context.get_symbol("to_s").to_object(context);
            auto end = context.get_symbol("end").to_object(context);
            auto block = context.get_symbol("block").to_object(context);

            if (from_s == context.get_symbol("from").to_object(context) && begin->type == Object::Int && to_s == context.get_symbol("to").to_object(context) && end->type == Object::Int) {
                for (long i = begin->data.i; i < end->data.i; i++) {
                    variable.get_reference() = context.new_object(i);
                    Interpreter::call_function(parent, nullptr, block->functions, std::make_shared<Tuple>());
                }

                return Reference(context.new_object());
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference for_step_statement(FunctionContext & context) {
            auto & parent = context.get_parent();

            auto variable = context.get_symbol("variable");
            auto from_s = context.get_symbol("from_s").to_object(context);
            auto begin = context.get_symbol("begin").to_object(context);
            auto to_s = context.get_symbol("to_s").to_object(context);
            auto end = context.get_symbol("end").to_object(context);
            auto step_s = context.get_symbol("step_s").to_object(context);
            auto step = context.get_symbol("step").to_object(context);
            auto block = context.get_symbol("block").to_object(context);

            if (from_s == context.get_symbol("from").to_object(context) && begin->type == Object::Int && to_s == context.get_symbol("to").to_object(context) && end->type == Object::Int && step_s == context.get_symbol("step").to_object(context) && step->type == Object::Int) {
                if (step->data.i > 0)
                    for (long i = begin->data.i; i < end->data.i; i += step->data.i) {
                        variable.get_reference() = context.new_object(i);
                        Interpreter::call_function(parent, nullptr, block->functions, std::make_shared<Tuple>());
                    }
                else if (step->data.i < 0)
                    for (long i = begin->data.i; i > end->data.i; i += step->data.i) {
                        variable.get_reference() = context.new_object(i);
                        Interpreter::call_function(parent, nullptr, block->functions, std::make_shared<Tuple>());
                    }
                else throw Interpreter::FunctionArgumentsError();

                return Reference(context.new_object());
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference try_statement(FunctionContext & context) {
            auto & parent = context.get_parent();

            auto try_block = context.get_symbol("try_block").to_object(context);
            auto catch_s = context.get_symbol("catch_s").to_object(context);
            auto catch_function = context.get_symbol("catch_function").to_object(context);

            if (catch_s == context.get_symbol("catch").to_object(context)) {
                try {
                    return Interpreter::call_function(parent, nullptr, try_block->functions, std::make_shared<Tuple>());
                } catch (Exception & ex) {
                    try {
                        return Interpreter::call_function(parent, nullptr, catch_function->functions, ex.reference);
                    } catch (Interpreter::Error & e) {
                        throw ex;
                    }
                }
            } else throw Interpreter::FunctionArgumentsError();
        }

        Reference throw_statement(FunctionContext & context) {
            Exception ex;
            ex.reference = context.get_symbol("throw_expression");
            ex.position = context.position;
            ex.position->store_stack_trace(context.get_parent());
            throw ex;
        }

        std::string get_canonical_path(FunctionContext & context) {
            std::string path;
            std::string system_position;
            try {
                path = context.get_symbol("path").to_object(context)->to_string();
                system_position = context.position->path;
            } catch (std::exception & e) {
                throw Interpreter::FunctionArgumentsError();
            }
            if (path[0] != '/')
                path = system_position.substr(0, system_position.find_last_of("/")+1) + path;
            std::filesystem::path p(path);
            return std::filesystem::canonical(p).string();
        }

        Reference include(FunctionContext & context) {
            std::string path = get_canonical_path(context);

            auto global = context.get_global();
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
                return Interpreter::run(global, expression);
            } catch (Parser::Standard::IncompleteCode & e) {
                std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
                return Reference(context.new_object());
            }
        }

        Reference use(FunctionContext & context) {
            std::string path = get_canonical_path(context);

            auto global = context.get_global();
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
                        global.get_symbol(symbol);

                    return Interpreter::run(global, expression);
                } catch (Parser::Standard::IncompleteCode & e) {
                    std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
                    return Reference(context.new_object());
                }
            } else
                return Reference(context.new_object());
        }

        Reference copy(FunctionContext & context) {
            auto object = context.get_symbol("object").to_object(context);

            if (object->type == Object::Char)
                return Reference(context.new_object(object->data.c));
            else if (object->type == Object::Float)
                return Reference(context.new_object(object->data.f));
            else if (object->type == Object::Int)
                return Reference(context.new_object(object->data.i));
            else if (object->type == Object::Bool)
                return Reference(context.new_object(object->data.b));
            else throw Interpreter::FunctionArgumentsError();
        }

        Reference copy_pointer(FunctionContext & context) {
            return Reference(context.get_symbol("object").to_object(context));
        }

        void assignation(Reference const& var, Object* const& object) {
            if (var.is_reference()) var.get_reference() = object;
            else if (var.type == object->type)
                for (long i = 0; i < var.type; i++)
                    assignation(var.tuple[i], object->data.a[i+1].o);
            else throw Interpreter::FunctionArgumentsError();
        }

        Reference assign(FunctionContext & context) {
            auto var = Interpreter::call_function(context, nullptr, context.get_symbol("var").to_object(context)->functions, std::make_shared<Tuple>());
            auto object = context.get_symbol("object").to_object(context);

            assignation(var, object);
            return var;
        }

        Reference function_definition(FunctionContext & context) {
            auto var = context.get_symbol("var").to_object(context);
            auto object = context.get_symbol("object").to_object(context);

            for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++) {
                if ((*it)->type == Function::Custom) {
                    var->functions.push_front(std::make_unique<CustomFunction>((CustomFunction&) **it));
                } else var->functions.push_front(std::make_unique<SystemFunction>((SystemFunction&) **it));
            }

            return context.get_symbol("var");
        }

        bool equals(Object* a, Object* b) {
            if (a->type == b->type) {
                if (a->type >= 0) {
                    for (auto const& element : a->properties) {
                        auto it = b->properties.find(element.first);
                        if (it != b->properties.end()) {
                            if (!equals(element.second, it->second))
                                return false;
                        } else return false;
                    }

                    auto ita = a->functions.begin();
                    auto itb = b->functions.begin();
                    while (ita != a->functions.end()) {
                        if (itb != b->functions.end()) {
                            if ((*ita)->type == Function::Custom) {
                                if (((CustomFunction&) **ita).pointer != ((CustomFunction&) **itb).pointer)
                                    return false;
                            } else
                                if (((SystemFunction&) **ita).pointer != ((SystemFunction&) **itb).pointer)
                                    return false;
                        } else return false;

                        ita++;
                        itb++;
                    }

                    for (long i = 1; i <= a->type; i++) {
                        if (!equals(a->data.a[i].o, b->data.a[i].o))
                            return false;
                    }
                    return true;
                } else if (a->type == Object::Bool)
                    return a->data.b == b->data.b;
                else if (a->type == Object::Int)
                    return a->data.i == b->data.i;
                else if (a->type == Object::Float)
                    return a->data.f == b->data.f;
                else if (a->type == Object::Char)
                    return a->data.c == b->data.c;
                else if (a->type == Object::CPointer)
                    return a->data.ptr == b->data.ptr;
                else return true;
            } else return false;
        }

        Reference equals(FunctionContext & context) {
            auto a = context.get_symbol("a").to_object(context);
            auto b = context.get_symbol("b").to_object(context);

            return Reference(context.new_object(equals(a, b)));
        }

        Reference not_equals(FunctionContext & context) {
            auto a = context.get_symbol("a").to_object(context);
            auto b = context.get_symbol("b").to_object(context);

            return Reference(context.new_object(!equals(a, b)));
        }

        Reference check_pointers(FunctionContext & context) {
            auto a = context.get_symbol("a").to_object(context);
            auto b = context.get_symbol("b").to_object(context);

            return Reference(context.new_object(a == b));
        }

        Reference not_check_pointers(FunctionContext & context) {
            auto a = context.get_symbol("a").to_object(context);
            auto b = context.get_symbol("b").to_object(context);

            return Reference(context.new_object(a != b));
        }

        void init(Context & context) {
            context.get_symbol(";").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("a"),
                std::make_shared<Symbol>("b")
            }), separator));

            auto if_s = std::make_unique<SystemFunction>(std::make_shared<FunctionCall>(
                std::make_shared<Symbol>("function"),
                std::make_shared<Tuple>()
            ), if_statement);
            if_s->extern_symbols["if"] = context.get_symbol("if");
            if_s->extern_symbols["else"] = context.get_symbol("else");
            context.get_symbol("if").to_object(context)->functions.push_front(std::move(if_s));

            context.get_symbol("while").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("condition"),
                    std::make_shared<Tuple>()
                ),
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("block"),
                    std::make_shared<Tuple>()
                )
            }), while_statement));

            auto for_s = std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("variable"),
                std::make_shared<Symbol>("from_s"),
                std::make_shared<Symbol>("begin"),
                std::make_shared<Symbol>("to_s"),
                std::make_shared<Symbol>("end"),
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("block"),
                    std::make_shared<Tuple>()
                )
            }), for_statement);
            for_s->extern_symbols["from"] = context.get_symbol("from");
            for_s->extern_symbols["to"] = context.get_symbol("to");
            context.get_symbol("for").to_object(context)->functions.push_front(std::move(for_s));

            auto for_step_s = std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
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
            }), for_step_statement);
            for_step_s->extern_symbols["from"] = context.get_symbol("from");
            for_step_s->extern_symbols["to"] = context.get_symbol("to");
            for_step_s->extern_symbols["step"] = context.get_symbol("step");
            context.get_symbol("for").to_object(context)->functions.push_front(std::move(for_step_s));

            auto try_s = std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("try_block"),
                    std::make_shared<Tuple>()
                ),
                std::make_shared<Symbol>("catch_s"),
                std::make_shared<Symbol>("catch_function"),
            }), try_statement);
            try_s->extern_symbols["catch"] = context.get_symbol("catch");
            context.get_symbol("try").to_object(context)->functions.push_front(std::move(try_s));

            auto equality = std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("a"),
                std::make_shared<Symbol>("b")
            });

            context.get_symbol("throw").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>("throw_expression"), throw_statement));
            context.get_symbol("include").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>("path"), include));
            context.get_symbol("using").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>("path"), use));
            context.get_symbol("$").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>("object"), copy));
            context.get_symbol("$==").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>("object"), copy_pointer));
            context.get_symbol(":=").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<FunctionCall>(
                    std::make_shared<Symbol>("var"),
                    std::make_shared<Tuple>()
                ),
                std::make_shared<Symbol>("object")
            }), assign));
            context.get_symbol(":").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Tuple>(std::vector<std::shared_ptr<Expression>> {
                std::make_shared<Symbol>("var"),
                std::make_shared<Symbol>("object")
            }), function_definition));
            context.get_symbol("==").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(equality, (Reference (*)(FunctionContext & context)) equals));
            context.get_symbol("!=").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(equality, not_equals));
            context.get_symbol("===").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(equality, check_pointers));
            context.get_symbol("!==").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(equality, not_check_pointers));
        }

    }

}
