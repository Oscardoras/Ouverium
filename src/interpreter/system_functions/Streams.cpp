#include <iostream>
#include <fstream>

#include "Streams.hpp"


namespace Interpreter {

    namespace Streams {

        Reference print(FunctionContext & context) {
            auto object = context["object"];

            Interpreter::print(std::cout, object);

            return Reference(context.new_object());
        }

        Reference scan(FunctionContext & context) {
            std::string str;
            getline(std::cin, str);

            long l = str.length();
            Object* obj = context.new_object();
            for (auto c : str)
                obj->array.push_back(c);
            return Reference(obj);
        }

        Reference read(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = (std::istream*) (*object)->c_pointer;

                std::string str;
                getline(*stream, str);

                long l = str.length();
                Object* obj = context.new_object();
                for (auto c : str)
                    obj->array.push_back(c);
                return Reference(obj);
            } else throw FunctionArgumentsError();
        }

        Reference has(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = (std::istream*) (*object)->c_pointer;
                return Reference(Data(stream->operator bool()));
            } else throw FunctionArgumentsError();
        }

        void setInputStream(Context & context, Object & object) {
            object.type = Object::CPointer;

            auto & read = object.properties["read"];
            if (read == nullptr) read = context.new_object();
            auto f1 = std::make_unique<SystemFunction>(std::make_shared<Tuple>(), Streams::read);
            f1->extern_symbols["this"] = Reference(&object);
            read->functions.push_front(std::move(f1));

            auto & has = object.properties["has"];
            if (has == nullptr) has = context.new_object();
            auto f2 = std::make_unique<SystemFunction>(std::make_shared<Tuple>(), Streams::has);
            f2->extern_symbols["this"] = Reference(&object);
            has->functions.push_front(std::move(f2));
        }

        Reference write(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = (std::ostream*) (*object)->c_pointer;
                auto message = context["message"];

                Interpreter::print(*stream, message);

                return Reference(context.new_object());
            } else throw FunctionArgumentsError();
        }

        Reference flush(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = (std::ostream*) (*object)->c_pointer;

                stream->flush();

                return Reference(context.new_object());
            } else throw FunctionArgumentsError();
        }

        void setOutputStream(Context & context, Object & object) {
            object.type = Object::CPointer;

            auto & write = object.properties["write"];
            if (write == nullptr) write = context.new_object();
            auto message = std::make_shared<Symbol>();
            message->name = "message";
            auto f1 = std::make_unique<SystemFunction>(message, Streams::write);
            f1->extern_symbols["this"] = Reference(&object);
            write->functions.push_front(std::move(f1));

            auto & flush = object.properties["flush"];
            if (flush == nullptr) flush = context.new_object();
            auto f2 = std::make_unique<SystemFunction>(std::make_shared<Tuple>(), Streams::flush);
            f2->extern_symbols["this"] = Reference(&object);
            flush->functions.push_front(std::move(f2));
        }

        Reference input_file(FunctionContext & context) {
            try {
                if (auto object = std::get_if<Object*>(context["path"])) {
                    auto path = (*object)->to_string();

                    auto object = context.new_object();
                    setInputStream(context, *object);
                    object->data.ptr = new std::ifstream(path);
                    context.get_global().c_pointers.push_back(object->data.ptr);

                    return Reference(object);
                }
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
                context.get_global().c_pointers.push_back(object->data.ptr);

                return Reference(object);
            } catch (std::exception & e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        void init(Context & context) {
            context["print"].to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>(
                "object"
            ), print));
            context.get_symbol("scan").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Tuple>(), scan));
            context.get_symbol("InputFile").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>(
                "path"
            ), input_file));
            context.get_symbol("OutputFile").to_object(context)->functions.push_front(std::make_unique<SystemFunction>(std::make_shared<Symbol>(
                "path"
            ), output_file));


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

}
