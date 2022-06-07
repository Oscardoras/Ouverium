#include <algorithm>
#include <iostream>
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
        return Reference(context.getSymbol("b"));
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
        auto function = ((CustomFunction*) *context.getSymbol("function").toObject(context)->functions.begin())->pointer->object;

        if (function->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(function);
            if (tuple->objects.size() >= 2) {
                auto parent = context.getParent();

                auto first_condition = Interpreter::execute(*parent, tuple->objects[0]).toObject(context);
                if (first_condition->type == Object::Bool) {
                    if (first_condition->data.b)
                        return Interpreter::execute(*parent, tuple->objects[1]).toObject(context);
                    else {
                        unsigned long i = 2;
                        while (i < tuple->objects.size()) {
                            auto else_s = Interpreter::execute(*parent, tuple->objects[i]).toObject(context);
                            if (else_s == context.getSymbol("else").toObject(context) && i+1 < tuple->objects.size()) {
                                auto s = Interpreter::execute(*parent, tuple->objects[i+1]).toObject(context);
                                if (s == context.getSymbol("if").toObject(context)) {
                                    if (i+3 < tuple->objects.size()) {
                                        auto condition = Interpreter::execute(*parent, tuple->objects[i+2]).toObject(context);
                                        if (condition->type == Object::Bool) {
                                            if (condition->data.b)
                                                return Interpreter::execute(*parent, tuple->objects[i+3]).toObject(context);
                                            else i += 4;
                                        } else throw FunctionArgumentsError();
                                    } else throw FunctionArgumentsError();
                                } else return s;
                            } else throw FunctionArgumentsError();
                        }
                        return Reference(context.newObject());
                    }
                } else throw FunctionArgumentsError();
            } else throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
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
        auto parent = context.getParent();
        Reference result;

        auto condition_functions = context.getSymbol("condition").toObject(context)->functions;
        auto block_functions = context.getSymbol("block").toObject(context)->functions;
        while (true) {
            auto condition = Interpreter::callFunction(*parent, condition_functions, std::make_shared<Tuple>(), nullptr).toObject(context);

            if (condition->type == Object::Bool) {
                if (condition->data.b) {
                    result = Interpreter::callFunction(*parent, block_functions, std::make_shared<Tuple>(), nullptr);
                } else break;
            } else throw FunctionArgumentsError();
        }
        
        if (result.type == Reference::Pointer && result.pointer == nullptr)
            return Reference(context.newObject());
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
        auto parent = context.getParent();

        auto variable = context.getSymbol("variable");
        auto from_s = context.getSymbol("from_s").toObject(context);
        auto begin = context.getSymbol("begin").toObject(context);
        auto to_s = context.getSymbol("to_s").toObject(context);
        auto end = context.getSymbol("end").toObject(context);
        auto block_functions = context.getSymbol("block").toObject(context)->functions;

        if (from_s == context.getSymbol("from_s").toObject(context) && begin->type == Object::Int && to_s == context.getSymbol("to_s").toObject(context) && end->type == Object::Int ) {
            for (long i = begin->data.i; i < end->data.i; i++) {
                variable.getReference() = context.newObject(i);
                Interpreter::callFunction(*parent, block_functions, std::make_shared<Tuple>(), nullptr);
            }
            
            return Reference(context.newObject());
        } else throw FunctionArgumentsError();
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
        auto parent = context.getParent();

        auto variable = context.getSymbol("variable");
        auto from_s = context.getSymbol("from_s").toObject(context);
        auto begin = context.getSymbol("begin").toObject(context);
        auto to_s = context.getSymbol("to_s").toObject(context);
        auto end = context.getSymbol("end").toObject(context);
        auto step_s = context.getSymbol("step_s").toObject(context);
        auto step = context.getSymbol("step").toObject(context);
        auto block_functions = context.getSymbol("block").toObject(context)->functions;

        if (from_s == context.getSymbol("from_s").toObject(context) && begin->type == Object::Int && to_s == context.getSymbol("to_s").toObject(context) && end->type == Object::Int && step_s == context.getSymbol("step_s").toObject(context) && step->type == Object::Int) {
            if (step->data.i > 0)
                for (long i = begin->data.i; i < end->data.i; i += step->data.i) {
                    variable.getReference() = context.newObject(i);
                    Interpreter::callFunction(*parent, block_functions, std::make_shared<Tuple>(), nullptr);
                }
            else if (step->data.i < 0)
                for (long i = begin->data.i; i > end->data.i; i += step->data.i) {
                    variable.getReference() = context.newObject(i);
                    Interpreter::callFunction(*parent, block_functions, std::make_shared<Tuple>(), nullptr);
                }
            else throw FunctionArgumentsError();
            
            return Reference(context.newObject());
        } else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> path() {
        auto path = std::make_shared<Symbol>();
        path->name = "path";
        return path;
    }

    Reference include(FunctionContext & context) {
        std::string path;
        std::string system_position;
        try {
            path = context.getSymbol("path").toObject(context)->toString();
            system_position = context.getSymbol("system_position").toObject(context)->toString();
        } catch (InterpreterError & e) {
            throw FunctionArgumentsError();
        }
        if (path[0] != '/')
            path = system_position.substr(0, system_position.find_last_of("/")+1) + path;

        std::ifstream file(path);

        std::string code;
        std::string line;
        while (std::getline(file, line))
            code += line + '\n';

        try {
            return Interpreter::run(*context.getGlobal(), path, code);
        } catch (StandardParser::IncompleteError & e) {
            std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
            return Reference(context.newObject());
        }
    }

    Reference use(FunctionContext & context) {
        std::string path;
        std::string system_position;
        try {
            path = context.getSymbol("path").toObject(context)->toString();
            system_position = context.getSymbol("system_position").toObject(context)->toString();
        } catch (InterpreterError & e) {
            throw FunctionArgumentsError();
        }
        if (path[0] != '/')
            path = system_position.substr(0, system_position.find_last_of("/")+1) + path;

        auto & files = context.getGlobal()->files;
        if (std::find(files.begin(), files.end(), path) == files.end()) {
            files.push_back(path);

            std::ifstream file(path);

            std::string code = "";
            std::string line = "";
            while (std::getline(file, line))
                code += line + '\n';

            try {
                return Interpreter::run(*context.getGlobal(), path, code);
            } catch (StandardParser::IncompleteError & e) {
                std::cerr << "incomplete code, you must finish the last expression in file \"" << path << "\"" << std::endl;
                return Reference(context.newObject());
            }
        } else return Reference(context.newObject());
    }

    std::shared_ptr<Expression> copy() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";

        return object;
    }
    Reference copy(FunctionContext & context) {
        auto object = context.getSymbol("object").toObject(context);

        if (object->type == Object::Char)
            return Reference(context.newObject(object->data.c));
        else if (object->type == Object::Float)
            return Reference(context.newObject(object->data.f));
        else if (object->type == Object::Int)
            return Reference(context.newObject(object->data.i));
        else if (object->type == Object::Bool)
            return Reference(context.newObject(object->data.b));
        else throw FunctionArgumentsError();
    }
    Reference copy_pointer(FunctionContext & context) {
        return Reference(context.getSymbol("object").toObject(context));
    }

    void assign1(std::vector<Object*> & cache, Reference const& var, Object* const& object) {
        if (var.type > 0) {
            if (var.type == object->type)
                for (long i = 0; i < var.type; i++) assign1(cache, var.tuple[i], object->data.a[i+1].o);
            else throw InterpreterError();
        } else cache.push_back(object);
    }

    void assign2(Context & context, std::vector<Object*>::iterator & it, Reference const& var) {
        if (var.isReference()) var.getReference() = *it++;
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
        auto var = Interpreter::callFunction(context, context.getSymbol("var").toObject(context)->functions, std::make_shared<Tuple>(), nullptr);
        auto object = Interpreter::callFunction(context, context.getSymbol("object").toObject(context)->functions, std::make_shared<Tuple>(), nullptr).toObject(context);

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
        auto var = context.getSymbol("var").toObject(context);
        auto object = context.getSymbol("object").toObject(context);

        for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++) {
            if ((*it)->type == Function::Custom) {
                var->functions.push_front(new CustomFunction(*((CustomFunction*) *it)));
            } else var->functions.push_front(new SystemFunction(*((SystemFunction*) *it)));
        }

        return context.getSymbol("var");
    }

    bool equals(Object* a, Object* b) {
        if (a->type == b->type) {
            if (a->type >= 0) {
                for (auto const& element : a->fields) {
                    auto it = b->fields.find(element.first);
                    if (it != b->fields.end()) {
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
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
        
        return Reference(context.newObject(equals(a, b)));
    }

    Reference not_equals(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);
        
        return Reference(context.newObject(!equals(a, b)));
    }

    Reference check_pointers(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        return Reference(context.newObject(a == b));
    }

    Reference not_check_pointers(FunctionContext & context) {
        auto a = context.getSymbol("a").toObject(context);
        auto b = context.getSymbol("b").toObject(context);

        return Reference(context.newObject(a != b));
    }

    void initiate(Context & context) {
        context.getSymbol(";").toObject(context)->functions.push_front(new SystemFunction(separator(), separator));

        auto if_s = new SystemFunction(if_statement(), if_statement);
        if_s->externSymbols["if"] = context.getSymbol("if");
        if_s->externSymbols["else"] = context.getSymbol("else");
        context.getSymbol("if").toObject(context)->functions.push_front(if_s);

        context.getSymbol("while").toObject(context)->functions.push_front(new SystemFunction(while_statement(), while_statement));

        auto for_s = new SystemFunction(for_statement(), for_statement);
        for_s->externSymbols["from"] = context.getSymbol("from");
        for_s->externSymbols["to"] = context.getSymbol("to");
        context.getSymbol("for").toObject(context)->functions.push_front(for_s);

        auto for_step_s = new SystemFunction(for_step_statement(), for_step_statement);
        for_step_s->externSymbols["from"] = context.getSymbol("from");
        for_step_s->externSymbols["to"] = context.getSymbol("to");
        for_step_s->externSymbols["step"] = context.getSymbol("step");
        context.getSymbol("for").toObject(context)->functions.push_front(for_step_s);

        context.getSymbol("include").toObject(context)->functions.push_front(new SystemFunction(path(), include));
        context.getSymbol("using").toObject(context)->functions.push_front(new SystemFunction(path(), use));
        context.getSymbol("$").toObject(context)->functions.push_front(new SystemFunction(copy(), copy));
        context.getSymbol("$==").toObject(context)->functions.push_front(new SystemFunction(copy(), copy_pointer));
        context.getSymbol(":=").toObject(context)->functions.push_front(new SystemFunction(assign(), assign));
        context.getSymbol(":").toObject(context)->functions.push_front(new SystemFunction(function_definition(), function_definition));
        context.getSymbol("==").toObject(context)->functions.push_front(new SystemFunction(equality(), equals));
        context.getSymbol("!=").toObject(context)->functions.push_front(new SystemFunction(equality(), not_equals));
        context.getSymbol("===").toObject(context)->functions.push_front(new SystemFunction(equality(), check_pointers));
        context.getSymbol("!==").toObject(context)->functions.push_front(new SystemFunction(equality(), not_check_pointers));
    }

}