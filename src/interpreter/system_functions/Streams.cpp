#include <iostream>
#include <fstream>

#include "../Interpreter.hpp"


namespace Streams {

    std::shared_ptr<Expression> print() {
        auto object = std::make_shared<Symbol>();
        object->name = "object";
        return object;
    }
    Reference print(FunctionContext & context) {
        auto object = context.getSymbol("object").toObject(context);

        Interpreter::print(std::cout, object);

        return Reference(context.newObject());
    }

    std::shared_ptr<Expression> scan() {
        return std::make_shared<Tuple>();
    }
    Reference scan(FunctionContext & context) {
        std::string str;
        getline(std::cin, str);

        long l = str.length();
        Object* obj = context.newObject((size_t) l);
        for (long i = 0; i < l; i++)
            obj->data.a[i+1].o = context.newObject(str[i]);
        return Reference(obj);
    }

    Reference read(FunctionContext & context) {
        auto stream = (std::istream*) context.getSymbol("this").toObject(context)->data.ptr;

        std::string str;
        getline(*stream, str);

        long l = str.length();
        Object* obj = context.newObject((size_t) l);
        for (long i = 0; i < l; i++)
            obj->data.a[i+1].o = context.newObject(str[i]);
        return Reference(obj);
    }

    Reference has(FunctionContext & context) {
        auto stream = (std::istream*) context.getSymbol("this").toObject(context)->data.ptr;

        return Reference(context.newObject(stream->operator bool()));
    }

    void setInputStream(Context & context, Object & object) {
        object.type = Object::CPointer;

        auto & read = object.fields["read"];
        if (read == nullptr) read = context.newObject();
        auto f1 = new SystemFunction(std::make_shared<Tuple>(), Streams::read);
        f1->externSymbols["this"] = Reference(&object);
        read->functions.push_front(f1);

        auto & has = object.fields["has"];
        if (has == nullptr) has = context.newObject();
        auto f2 = new SystemFunction(std::make_shared<Tuple>(), Streams::has);
        f2->externSymbols["this"] = Reference(&object);
        has->functions.push_front(f2);
    }

    Reference write(FunctionContext & context) {
        auto stream = (std::ostream*) context.getSymbol("this").toObject(context)->data.ptr;
        auto message = context.getSymbol("message").toObject(context);

        Interpreter::print(*stream, message);

        return Reference(context.newObject());
    }

    Reference flush(FunctionContext & context) {
        auto stream = (std::ostream*) context.getSymbol("this").toObject(context)->data.ptr;

        stream->flush();

        return Reference(context.newObject());
    }

    void setOutputStream(Context & context, Object & object) {
        object.type = Object::CPointer;

        auto & write = object.fields["write"];
        if (write == nullptr) write = context.newObject();
        auto message = std::make_shared<Symbol>();
        message->name = "message";
        auto f = new SystemFunction(message, Streams::write);
        f->externSymbols["this"] = Reference(&object);
        write->functions.push_front(f);

        auto & flush = object.fields["flush"];
        if (flush == nullptr) flush = context.newObject();
        auto f2 = new SystemFunction(std::make_shared<Tuple>(), Streams::flush);
        f2->externSymbols["this"] = Reference(&object);
        flush->functions.push_front(f2);
    }

    std::shared_ptr<Expression> path() {
        auto path = std::make_shared<Symbol>();
        path->name = "path";
        return path;
    }
    Reference input_file(FunctionContext & context) {
        try {
            auto path = context.getSymbol("path").toObject(context)->toString();

            auto object = context.newObject();
            setInputStream(context, *object);
            object->data.ptr = new std::ifstream(path);
            context.getGlobal()->cpointers.push_back(object->data.ptr);

            return Reference(object);
        } catch (InterpreterError & e) {
            throw FunctionArgumentsError();
        }
    }
    Reference output_file(FunctionContext & context) {
        try {
            auto path = context.getSymbol("this").toObject(context)->toString();

            auto object = context.newObject();
            setOutputStream(context, *object);
            object->data.ptr = new std::ofstream(path);
            context.getGlobal()->cpointers.push_back(object->data.ptr);

            return Reference(object);
        } catch (InterpreterError & e) {
            throw FunctionArgumentsError();
        }
    }

    void initiate(Context & context) {
        context.getSymbol("print").toObject(context)->functions.push_front(new SystemFunction(print(), print));
        context.getSymbol("scan").toObject(context)->functions.push_front(new SystemFunction(scan(), scan));
        context.getSymbol("InputFile").toObject(context)->functions.push_front(new SystemFunction(path(), input_file));
        context.getSymbol("OutputFile").toObject(context)->functions.push_front(new SystemFunction(path(), output_file));


        auto* console = context.newObject();
        context.getSymbol("Console").getReference() = console;

        auto in = context.newObject();
        setInputStream(context, *in);
        in->data.ptr = &std::cin;
        console->fields["in"] = in;

        auto out = context.newObject();
        setOutputStream(context, *out);
        out->data.ptr = &std::cout;
        console->fields["out"] = out;

        auto err = context.newObject();
        setOutputStream(context, *err);
        err->data.ptr = &std::cerr;
        console->fields["err"] = err;
    }

}