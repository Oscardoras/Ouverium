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
        auto object = context.get_symbol("object").to_object(context);

        Interpreter::print(std::cout, object);

        return Reference(context.new_object());
    }

    std::shared_ptr<Expression> scan() {
        return std::make_shared<Tuple>();
    }
    Reference scan(FunctionContext & context) {
        std::string str;
        getline(std::cin, str);

        long l = str.length();
        Object* obj = context.new_object((size_t) l);
        for (long i = 0; i < l; i++)
            obj->data.a[i+1].o = context.new_object(str[i]);
        return Reference(obj);
    }

    Reference read(FunctionContext & context) {
        auto stream = (std::istream*) context.get_symbol("this").to_object(context)->data.ptr;

        std::string str;
        getline(*stream, str);

        long l = str.length();
        Object* obj = context.new_object((size_t) l);
        for (long i = 0; i < l; i++)
            obj->data.a[i+1].o = context.new_object(str[i]);
        return Reference(obj);
    }

    Reference has(FunctionContext & context) {
        auto stream = (std::istream*) context.get_symbol("this").to_object(context)->data.ptr;

        return Reference(context.new_object(stream->operator bool()));
    }

    void setInputStream(Context & context, Object & object) {
        object.type = Object::CPointer;

        auto & read = object.properties["read"];
        if (read == nullptr) read = context.new_object();
        auto f1 = new SystemFunction(std::make_shared<Tuple>(), Streams::read);
        f1->extern_symbols["this"] = Reference(&object);
        read->functions.push_front(f1);

        auto & has = object.properties["has"];
        if (has == nullptr) has = context.new_object();
        auto f2 = new SystemFunction(std::make_shared<Tuple>(), Streams::has);
        f2->extern_symbols["this"] = Reference(&object);
        has->functions.push_front(f2);
    }

    Reference write(FunctionContext & context) {
        auto stream = (std::ostream*) context.get_symbol("this").to_object(context)->data.ptr;
        auto message = context.get_symbol("message").to_object(context);

        Interpreter::print(*stream, message);

        return Reference(context.new_object());
    }

    Reference flush(FunctionContext & context) {
        auto stream = (std::ostream*) context.get_symbol("this").to_object(context)->data.ptr;

        stream->flush();

        return Reference(context.new_object());
    }

    void setOutputStream(Context & context, Object & object) {
        object.type = Object::CPointer;

        auto & write = object.properties["write"];
        if (write == nullptr) write = context.new_object();
        auto message = std::make_shared<Symbol>();
        message->name = "message";
        auto f = new SystemFunction(message, Streams::write);
        f->extern_symbols["this"] = Reference(&object);
        write->functions.push_front(f);

        auto & flush = object.properties["flush"];
        if (flush == nullptr) flush = context.new_object();
        auto f2 = new SystemFunction(std::make_shared<Tuple>(), Streams::flush);
        f2->extern_symbols["this"] = Reference(&object);
        flush->functions.push_front(f2);
    }

    std::shared_ptr<Expression> path() {
        auto path = std::make_shared<Symbol>();
        path->name = "path";
        return path;
    }
    Reference input_file(FunctionContext & context) {
        try {
            auto path = context.get_symbol("path").to_object(context)->to_string();

            auto object = context.new_object();
            setInputStream(context, *object);
            object->data.ptr = new std::ifstream(path);
            context.get_global()->c_pointers.push_back(object->data.ptr);

            return Reference(object);
        } catch (std::exception & e) {
            throw Interpreter::FunctionArgumentsError();
        }
    }
    Reference output_file(FunctionContext & context) {
        try {
            auto path = context.get_symbol("path").to_object(context)->to_string();

            auto object = context.new_object();
            setOutputStream(context, *object);
            object->data.ptr = new std::ofstream(path);
            context.get_global()->c_pointers.push_back(object->data.ptr);

            return Reference(object);
        } catch (std::exception & e) {
            throw Interpreter::FunctionArgumentsError();
        }
    }

    void initiate(Context & context) {
        context.get_symbol("print").to_object(context)->functions.push_front(new SystemFunction(print(), print));
        context.get_symbol("scan").to_object(context)->functions.push_front(new SystemFunction(scan(), scan));
        context.get_symbol("InputFile").to_object(context)->functions.push_front(new SystemFunction(path(), input_file));
        context.get_symbol("OutputFile").to_object(context)->functions.push_front(new SystemFunction(path(), output_file));


        auto console = context.get_symbol("Console").to_object(context);

        auto in = context.new_object();
        setInputStream(context, *in);
        in->data.ptr = &std::cin;
        console->properties["in"] = in;

        auto out = context.new_object();
        setOutputStream(context, *out);
        out->data.ptr = &std::cout;
        console->properties["out"] = out;

        auto err = context.new_object();
        setOutputStream(context, *err);
        err->data.ptr = &std::cerr;
        console->properties["err"] = err;
    }

}
