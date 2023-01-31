#include <iostream>
#include <fstream>

#include "Streams.hpp"


namespace Interpreter {

    namespace Streams {

        auto print_args = std::make_shared<Symbol>("data");
        Reference print(FunctionContext & context) {
            auto data = context["data"];
            Interpreter::print(std::cout, data);
            return Reference(context.new_object());
        }

        auto scan_args = std::make_shared<Tuple>();
        Reference scan(FunctionContext & context) {
            std::string str;
            getline(std::cin, str);

            long l = str.length();
            Object* obj = context.new_object();
            for (auto c : str)
                obj->array.push_back(c);
            return Reference(Data(obj));
        }

        auto read_args = std::make_shared<Tuple>();
        Reference read(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = dynamic_cast<std::ifstream*>((*object)->c_pointer.get());

                std::string str;
                getline(*stream, str);

                long l = str.length();
                Object* obj = context.new_object();
                for (auto c : str)
                    obj->array.push_back(c);
                return Reference(obj);
            } else throw FunctionArgumentsError();
        }

        auto has_args = std::make_shared<Tuple>();
        Reference has(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = dynamic_cast<std::ifstream*>((*object)->c_pointer.get());
                return Reference(Data(stream->operator bool()));
            } else throw FunctionArgumentsError();
        }

        void setInputStream(Context & context, Object & object) {
            auto & read = object.get_property("read", context);
            Function f1 = SystemFunction{std::make_shared<Tuple>(), Streams::read};
            f1.extern_symbols["this"] = &object;
            read->functions.push_front(std::move(f1));

            auto & has = object.properties["has"];
            if (has == nullptr) has = context.new_object();
            auto f2 = std::make_unique<SystemFunction>(std::make_shared<Tuple>(), Streams::has);
            f2->extern_symbols["this"] = Reference(&object);
            has->functions.push_front(std::move(f2));
        }

        auto print_args = std::make_shared<Symbol>("data");
        Reference write(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = (std::ostream*) (*object)->c_pointer;
                auto data = context["data"];

                Interpreter::print(*stream, data);

                return Reference(context.new_object());
            } else throw FunctionArgumentsError();
        }

        auto flush_args = std::make_shared<Tuple>();
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

        auto input_file_args = std::make_shared<Symbol>("path");
        Reference input_file(FunctionContext & context) {
            try {
                if (auto path_object = std::get_if<Object*>(&context["path"])) {
                    auto path = (*path_object)->to_string();

                    auto object = context.new_object();
                    setInputStream(context, *object);
                    object->c_pointer = std::make_unique<std::ifstream>(path);

                    return Reference(Data(object));
                } else FunctionArgumentsError();
            } catch (std::exception & e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        auto output_file_args = std::make_shared<Symbol>("path");
        Reference output_file(FunctionContext & context) {
            try {
                if (auto path_object = std::get_if<Object*>(&context["path"])) {
                    auto path = (*path_object)->to_string();

                    auto object = context.new_object();
                    setOutputStream(context, *object);
                    object->c_pointer = std::make_unique<std::ofstream>(path);

                    return Reference(object);
                    } else FunctionArgumentsError();
            } catch (std::exception & e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        void init(Context & context) {
            context.get_function("print").push_front(SystemFunction{print_args, print});
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
