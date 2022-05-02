#include <algorithm>
#include <iostream>
#include <fstream>

#include "../Interpreter.hpp"


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
        auto tuple = std::make_shared<Tuple>();

        auto condition = std::make_shared<Symbol>();
        condition->name = "condition";
        tuple->objects.push_back(condition);

        auto block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        return tuple;
    }
    Reference if_statement(FunctionContext & context) {
        auto condition = context.getSymbol("condition").toObject(context);

        if (condition->type == Object::Bool)
            if (condition->data.b) {
                auto parent = context.getParent();
                auto functions = context.getSymbol("block").toObject(context)->functions;
                return Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>(), nullptr);
            } else return Reference(context.newObject());
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> if_else_statement() {
        auto tuple = std::make_shared<Tuple>();

        auto condition = std::make_shared<Symbol>();
        condition->name = "condition";
        tuple->objects.push_back(condition);

        auto block = std::make_shared<FunctionCall>();
        auto function_name = std::make_shared<Symbol>();
        function_name->name = "block";
        block->function = function_name;
        block->object = std::make_shared<Tuple>();
        tuple->objects.push_back(block);

        auto else_s = std::make_shared<Symbol>();
        else_s->name = "else_s";
        tuple->objects.push_back(else_s);

        auto alternative = std::make_shared<FunctionCall>();
        function_name = std::make_shared<Symbol>();
        function_name->name = "alternative";
        alternative->function = function_name;
        alternative->object = std::make_shared<Tuple>();
        tuple->objects.push_back(alternative);

        return tuple;
    }
    Reference if_else_statement(FunctionContext & context) {
        auto condition = context.getSymbol("condition").toObject(context);

        if (condition->type == Object::Bool && context.getSymbol("else_s").toObject(context) == context.getSymbol("else").toObject(context))
            if (condition->data.b) {
                auto parent = context.getParent();
                auto functions = context.getSymbol("block").toObject(context)->functions;
                return Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>(), nullptr);
            } else {
                auto parent = context.getParent();
                auto functions = context.getSymbol("alternative").toObject(context)->functions;
                return Interpreter::callFunction(*parent, functions, std::make_shared<Tuple>(), nullptr);
            }
        else throw FunctionArgumentsError();
    }

    std::shared_ptr<Expression> else_statement() {
        return std::make_shared<Tuple>();
    }
    Reference else_statement(__attribute__((unused)) FunctionContext & context) {
        throw FunctionArgumentsError();
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

    std::shared_ptr<Expression> path() {
        auto path = std::make_shared<Symbol>();
        path->name = "path";
        return path;
    }

    Reference include(FunctionContext & context) {
        auto path = context.getSymbol("path").toObject(context);
        auto system_position = context.getSymbol("system_position").toObject(context);

        std::string path_name;
        try {
            path_name = path->toString();
        } catch (InterpreterError e) {
            throw FunctionArgumentsError();
        }

        std::ifstream file(path_name);

        std::string code = "";
        std::string line = "";
        while (std::getline(file, line))
            code += line + " ";

        return Interpreter::run(context, path_name, code);
    }

    Reference use(FunctionContext & context) {
        auto path = context.getSymbol("path").toObject(context);

        std::string path_name;
        try {
            path_name = path->toString();
        } catch (InterpreterError e) {
            throw FunctionArgumentsError();
        }

        auto & files = context.getGlobal()->files;
        if (std::find(files.begin(), files.end(), path_name) != files.end()) {
            files.push_back(path_name);

            std::ifstream file(path_name);

            std::string code = "";
            std::string line = "";
            while (std::getline(file, line))
                code += line + " ";

            return Interpreter::run(context, , code);
        } else return Reference(context.newObject());
    }

    std::shared_ptr<Expression> copy() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";

        return object;
    }
    Reference copy(FunctionContext & context) {
        auto object = context.getSymbol("object").toObject(context);
        return Reference(context.newObject(*object));
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

            if (a->type >= 0) {
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
        context.getSymbol("if").toObject(context)->functions.push_front(new SystemFunction(if_statement(), if_statement));
        context.getSymbol("if").toObject(context)->functions.push_front(new SystemFunction(if_else_statement(), if_else_statement));
        context.getSymbol("else").toObject(context)->functions.push_front(new SystemFunction(else_statement(), else_statement));
        context.getSymbol("while").toObject(context)->functions.push_front(new SystemFunction(while_statement(), while_statement));
        context.getSymbol("include").toObject(context)->functions.push_front(new SystemFunction(path(), include));
        context.getSymbol("using").toObject(context)->functions.push_front(new SystemFunction(path(), use));
        context.getSymbol("$").toObject(context)->functions.push_front(new SystemFunction(copy(), copy));
        context.getSymbol(":=").toObject(context)->functions.push_front(new SystemFunction(assign(), assign));
        context.getSymbol(":").toObject(context)->functions.push_front(new SystemFunction(function_definition(), function_definition));
        context.getSymbol("==").toObject(context)->functions.push_front(new SystemFunction(equality(), equals));
        context.getSymbol("!=").toObject(context)->functions.push_front(new SystemFunction(equality(), not_equals));
        context.getSymbol("===").toObject(context)->functions.push_front(new SystemFunction(equality(), check_pointers));
        context.getSymbol("!==").toObject(context)->functions.push_front(new SystemFunction(equality(), not_check_pointers));
    }

}