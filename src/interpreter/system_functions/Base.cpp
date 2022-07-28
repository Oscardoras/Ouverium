#include <algorithm>
#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "../Interpreter.hpp"
#include "../../parser/Standard.hpp"


namespace Base {

    std::shared_ptr<Expression> separator() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
    }
    Reference separator(FunctionContext & context) {
        return Reference(context.get_symbol("b"));
    }

    std::shared_ptr<Expression> if_statement() {
        auto function = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "function";
        function->function = function_name;
        function->object = std::make_shared<Tuple>();
        return function;
    }
    Reference if_statement(FunctionContext & context) {
        auto function = ((CustomFunction*) *context.get_symbol("function").to_object(context)->functions.begin())->pointer->object;

        if (function->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(function);
            if (tuple->objects.size() >= 2) {
                auto parent = context.get_parent();

                auto first_condition = Interpreter::execute(*parent, tuple->objects[0]).to_object(context);
                if (first_condition->type == Object::Bool) {
                    if (first_condition->data.b)
                        return Interpreter::execute(*parent, tuple->objects[1]).to_object(context);
                    else {
                        unsigned long i = 2;
                        while (i < tuple->objects.size()) {
                            auto else_s = Interpreter::execute(*parent, tuple->objects[i]).to_object(context);
                            if (else_s == context.get_symbol("else").to_object(context) && i+1 < tuple->objects.size()) {
                                auto s = Interpreter::execute(*parent, tuple->objects[i+1]).to_object(context);
                                if (s == context.get_symbol("if").to_object(context)) {
                                    if (i+3 < tuple->objects.size()) {
                                        auto condition = Interpreter::execute(*parent, tuple->objects[i+2]).to_object(context);
                                        if (condition->type == Object::Bool) {
                                            if (condition->data.b)
                                                return Interpreter::execute(*parent, tuple->objects[i+3]).to_object(context);
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

    std::shared_ptr<Expression> while_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto condition = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "condition";
        condition->function = function_name;
        condition->object = std::make_shared<Tuple>();
        tuple->objects.push_back(condition);

        auto block = std::make_shared<FunctionCall>();
        function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        return tuple;
    }
    Reference while_statement(FunctionContext & context) {
        auto parent = context.get_parent();
        Reference result;

        auto condition_functions = context.get_symbol("condition").to_object(context)->functions;
        auto block_functions = context.get_symbol("block").to_object(context)->functions;
        while (true) {
            auto condition = Interpreter::call_function(*parent, nullptr, condition_functions, std::make_shared<Tuple>()).to_object(context);

            if (condition->type == Object::Bool) {
                if (condition->data.b) {
                    result = Interpreter::call_function(*parent, nullptr, block_functions, std::make_shared<Tuple>());
                } else break;
            } else throw Interpreter::FunctionArgumentsError();
        }
        
        if (result.type == Reference::Pointer && result.pointer == nullptr)
            return Reference(context.new_object());
        else return result;
    }

    std::shared_ptr<Expression> for_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto variable = std::make_shared<Symbol>();
        variable->name = "variable";
        tuple->objects.push_back(variable);

        auto from_s = std::make_shared<Symbol>();
        from_s->name = "from_s";
        tuple->objects.push_back(from_s);

        auto begin = std::make_shared<Symbol>();
        begin->name = "begin";
        tuple->objects.push_back(begin);

        auto to_s = std::make_shared<Symbol>();
        to_s->name = "to_s";
        tuple->objects.push_back(to_s);

        auto end = std::make_shared<Symbol>();
        end->name = "end";
        tuple->objects.push_back(end);

        auto block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        return tuple;
    }
    Reference for_statement(FunctionContext & context) {
        auto parent = context.get_parent();

        auto variable = context.get_symbol("variable");
        auto from_s = context.get_symbol("from_s").to_object(context);
        auto begin = context.get_symbol("begin").to_object(context);
        auto to_s = context.get_symbol("to_s").to_object(context);
        auto end = context.get_symbol("end").to_object(context);
        auto block_functions = context.get_symbol("block").to_object(context)->functions;

        if (from_s == context.get_symbol("from").to_object(context) && begin->type == Object::Int && to_s == context.get_symbol("to").to_object(context) && end->type == Object::Int) {
            for (long i = begin->data.i; i < end->data.i; i++) {
                variable.get_reference() = context.new_object(i);
                Interpreter::call_function(*parent, nullptr, block_functions, std::make_shared<Tuple>());
            }
            
            return Reference(context.new_object());
        } else throw Interpreter::FunctionArgumentsError();
    }

    std::shared_ptr<Expression> for_step_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto variable = std::make_shared<Symbol>();
        variable->name = "variable";
        tuple->objects.push_back(variable);

        auto from_s = std::make_shared<Symbol>();
        from_s->name = "from_s";
        tuple->objects.push_back(from_s);

        auto begin = std::make_shared<Symbol>();
        begin->name = "begin";
        tuple->objects.push_back(begin);

        auto to_s = std::make_shared<Symbol>();
        to_s->name = "to_s";
        tuple->objects.push_back(to_s);

        auto end = std::make_shared<Symbol>();
        end->name = "end";
        tuple->objects.push_back(end);

        auto step_s = std::make_shared<Symbol>();
        step_s->name = "step_s";
        tuple->objects.push_back(step_s);

        auto step = std::make_shared<Symbol>();
        step->name = "step";
        tuple->objects.push_back(step);

        auto block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        return tuple;
    }
    Reference for_step_statement(FunctionContext & context) {
        auto parent = context.get_parent();

        auto variable = context.get_symbol("variable");
        auto from_s = context.get_symbol("from_s").to_object(context);
        auto begin = context.get_symbol("begin").to_object(context);
        auto to_s = context.get_symbol("to_s").to_object(context);
        auto end = context.get_symbol("end").to_object(context);
        auto step_s = context.get_symbol("step_s").to_object(context);
        auto step = context.get_symbol("step").to_object(context);
        auto block_functions = context.get_symbol("block").to_object(context)->functions;

        if (from_s == context.get_symbol("from").to_object(context) && begin->type == Object::Int && to_s == context.get_symbol("to").to_object(context) && end->type == Object::Int && step_s == context.get_symbol("step").to_object(context) && step->type == Object::Int) {
            if (step->data.i > 0)
                for (long i = begin->data.i; i < end->data.i; i += step->data.i) {
                    variable.get_reference() = context.new_object(i);
                    Interpreter::call_function(*parent, nullptr, block_functions, std::make_shared<Tuple>());
                }
            else if (step->data.i < 0)
                for (long i = begin->data.i; i > end->data.i; i += step->data.i) {
                    variable.get_reference() = context.new_object(i);
                    Interpreter::call_function(*parent, nullptr, block_functions, std::make_shared<Tuple>());
                }
            else throw Interpreter::FunctionArgumentsError();
            
            return Reference(context.new_object());
        } else throw Interpreter::FunctionArgumentsError();
    }

    struct Exception {
        Reference reference;
        std::shared_ptr<Position> position;
    };

    std::shared_ptr<Expression> try_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto try_block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "try_block";
        try_block->function = function_name;
        try_block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(try_block);

        auto catch_s = std::make_shared<Symbol>();
        catch_s->name = "catch_s";
        tuple->objects.push_back(catch_s);

        auto catch_function = std::make_shared<Symbol>();
        catch_function->name = "catch_function";
        tuple->objects.push_back(catch_function);

        return tuple;
    }
    Reference try_statement(FunctionContext & context) {
        auto parent = context.get_parent();

        auto try_block = context.get_symbol("try_block").to_object(context);
        auto catch_s = context.get_symbol("catch_s").to_object(context);
        auto catch_function = context.get_symbol("catch_function").to_object(context);

        if (catch_s == context.get_symbol("catch").to_object(context)) {
            try {
                return Interpreter::call_function(*parent, nullptr, try_block->functions, std::make_shared<Tuple>());
            } catch (Exception & ex) {
                try {
                    return Interpreter::call_function(*parent, nullptr, catch_function->functions, ex.reference);
                } catch (Interpreter::Error & e) {
                    throw ex;
                }
            }
        } else throw Interpreter::FunctionArgumentsError();
    }

    std::shared_ptr<Expression> throw_statement() {
        auto throw_expression = std::make_shared<Symbol>();
        throw_expression->name = "throw_expression";
        return throw_expression;
    }
    Reference throw_statement(FunctionContext & context) {
        Exception ex;
        ex.reference = context.get_symbol("throw_expression");
        ex.position = context.position;
        ex.position->get_stack_trace(*context.get_parent());
        throw ex;
    }

    std::string get_canonical_path(FunctionContext & context) {
        std::string path;
        std::string system_position;
        try {
            path = context.get_symbol("path").to_object(context)->to_string();
            system_position = context.get_symbol("system_position").to_object(context)->to_string();
        } catch (std::exception & e) {
            throw Interpreter::FunctionArgumentsError();
        }
        if (path[0] != '/')
            path = system_position.substr(0, system_position.find_last_of("/")+1) + path;
        std::filesystem::path p(path);
        return std::filesystem::canonical(p).string();
    }

    std::shared_ptr<Expression> path() {
        auto path = std::make_shared<Symbol>();
        path->name = "path";
        return path;
    }

    Reference include(FunctionContext & context) {
        std::string path = get_canonical_path(context);

        auto global = context.get_global();
        std::vector<std::string> symbols;
        for (auto it = global->symbols.begin(); it != global->symbols.end(); it++)
            symbols.push_back(it->first);

        std::ifstream file(path);
        std::string code;
        std::string line;
        while (std::getline(file, line))
            code += line + '\n';
        
        try {
            auto expression = StandardParser::get_tree(code, path, symbols);
            return Interpreter::run(*global, expression);
        } catch (StandardParser::IncompleteCode & e) {
            std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
            return Reference(context.new_object());
        }
    }

    Reference use(FunctionContext & context) {
        std::string path = get_canonical_path(context);

        auto global = context.get_global();
        if (global->files.find(path) == global->files.end()) {
            std::vector<std::string> symbols;
            for (auto it = global->symbols.begin(); it != global->symbols.end(); it++)
                symbols.push_back(it->first);

            std::ifstream file(path);
            std::string code;
            std::string line;
            while (std::getline(file, line))
                code += line + '\n';
            
            try {
                auto expression = StandardParser::get_tree(code, path, symbols);
                global->files[path] = expression;

                for (auto const& symbol : expression->symbols)
                    global->get_symbol(symbol);
                    
                return Interpreter::run(*global, expression);
            } catch (StandardParser::IncompleteCode & e) {
                std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
                return Reference(context.new_object());
            }
        } else
            return Reference(context.new_object());
    }

    std::shared_ptr<Expression> copy() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";

        return object;
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

    void assign1(std::vector<Object*> & cache, Reference const& var, Object* const& object) {
        if (var.type > 0) {
            if (var.type == object->type)
                for (long i = 0; i < var.type; i++) assign1(cache, var.tuple[i], object->data.a[i+1].o);
            else throw std::exception();
        } else cache.push_back(object);
    }

    void assign2(Context & context, std::vector<Object*>::iterator & it, Reference const& var) {
        if (var.is_reference()) var.get_reference() = *it++;
        else if (var.type > 0) {
            for (long i = 0; i < var.type; i++) assign2(context, it, var.tuple[i]);
        }
    }

    std::shared_ptr<Expression> assign() {
        auto tuple = std::make_shared<Tuple>();

        auto var = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "var";
        var->function = function_name;
        var->object = std::make_shared<Tuple>();
        tuple->objects.push_back(var);

        auto object = std::make_shared<FunctionCall>();
        function_name = std::make_shared<Symbol>();
        function_name->name = "object";
        object->function = function_name;
        object->object = std::make_shared<Tuple>();
        tuple->objects.push_back(object);

        return tuple;
    }
    Reference assign(FunctionContext & context) {
        auto var = Interpreter::call_function(context, nullptr, context.get_symbol("var").to_object(context)->functions, std::make_shared<Tuple>());
        auto object = Interpreter::call_function(context, nullptr, context.get_symbol("object").to_object(context)->functions, std::make_shared<Tuple>()).to_object(context);

        std::vector<Object*> cache;
        assign1(cache, var, object);
        auto it = cache.begin();
        assign2(context, it, var);

        return var;
    }

    std::shared_ptr<Expression> function_definition() {
        auto tuple = std::make_shared<Tuple>();

        auto var = std::make_shared<Symbol>();
        var->name = "var";
        tuple->objects.push_back(var);

        auto object = std::make_shared<Symbol>();
        object->name = "object";
        tuple->objects.push_back(object);

        return tuple;
    }
    Reference function_definition(FunctionContext & context) {
        auto var = context.get_symbol("var").to_object(context);
        auto object = context.get_symbol("object").to_object(context);

        for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++) {
            if ((*it)->type == Function::Custom) {
                var->functions.push_front(new CustomFunction(*((CustomFunction*) *it)));
            } else var->functions.push_front(new SystemFunction(*((SystemFunction*) *it)));
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
                            if (((CustomFunction*) *ita)->pointer != ((CustomFunction*) *itb)->pointer)
                                return false;
                        } else
                            if (((SystemFunction*) *ita)->pointer != ((SystemFunction*) *itb)->pointer)
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

    std::shared_ptr<Expression> equality() {
        auto tuple = std::make_shared<Tuple>();

        auto a = std::make_shared<Symbol>();
        a->name = "a";
        tuple->objects.push_back(a);

        auto b = std::make_shared<Symbol>();
        b->name = "b";
        tuple->objects.push_back(b);

        return tuple;
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

    void initiate(Context & context) {
        context.get_symbol(";").to_object(context)->functions.push_front(new SystemFunction(separator(), separator));

        auto if_s = new SystemFunction(if_statement(), if_statement);
        if_s->extern_symbols["if"] = context.get_symbol("if");
        if_s->extern_symbols["else"] = context.get_symbol("else");
        context.get_symbol("if").to_object(context)->functions.push_front(if_s);

        context.get_symbol("while").to_object(context)->functions.push_front(new SystemFunction(while_statement(), while_statement));

        auto for_s = new SystemFunction(for_statement(), for_statement);
        for_s->extern_symbols["from"] = context.get_symbol("from");
        for_s->extern_symbols["to"] = context.get_symbol("to");
        context.get_symbol("for").to_object(context)->functions.push_front(for_s);

        auto for_step_s = new SystemFunction(for_step_statement(), for_step_statement);
        for_step_s->extern_symbols["from"] = context.get_symbol("from");
        for_step_s->extern_symbols["to"] = context.get_symbol("to");
        for_step_s->extern_symbols["step"] = context.get_symbol("step");
        context.get_symbol("for").to_object(context)->functions.push_front(for_step_s);

        auto try_s = new SystemFunction(try_statement(), try_statement);
        try_s->extern_symbols["catch"] = context.get_symbol("catch");
        context.get_symbol("try").to_object(context)->functions.push_front(try_s);

        context.get_symbol("throw").to_object(context)->functions.push_front(new SystemFunction(throw_statement(), throw_statement));

        context.get_symbol("include").to_object(context)->functions.push_front(new SystemFunction(path(), include));
        context.get_symbol("using").to_object(context)->functions.push_front(new SystemFunction(path(), use));
        context.get_symbol("$").to_object(context)->functions.push_front(new SystemFunction(copy(), copy));
        context.get_symbol("$==").to_object(context)->functions.push_front(new SystemFunction(copy(), copy_pointer));
        context.get_symbol(":=").to_object(context)->functions.push_front(new SystemFunction(assign(), assign));
        context.get_symbol(":").to_object(context)->functions.push_front(new SystemFunction(function_definition(), function_definition));
        context.get_symbol("==").to_object(context)->functions.push_front(new SystemFunction(equality(), equals));
        context.get_symbol("!=").to_object(context)->functions.push_front(new SystemFunction(equality(), not_equals));
        context.get_symbol("===").to_object(context)->functions.push_front(new SystemFunction(equality(), check_pointers));
        context.get_symbol("!==").to_object(context)->functions.push_front(new SystemFunction(equality(), not_check_pointers));
    }

}
