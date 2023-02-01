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
            Function f1 = SystemFunction{read_args, Streams::read};
            f1.extern_symbols["this"] = context.new_reference(&object);
            std::get<Object*>(object.get_property("read", context))->functions.push_front(f1);

            Function f2 = SystemFunction{has_args, Streams::has};
            f2.extern_symbols["this"] = context.new_reference(&object);
            std::get<Object*>(object.get_property("has", context))->functions.push_front(f2);
        }

        auto write_args = std::make_shared<Symbol>("data");
        Reference write(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = dynamic_cast<std::ostream*>((*object)->c_pointer.get());
                auto data = context["data"];

                Interpreter::print(*stream, data);

                return Reference(context.new_object());
            } else throw FunctionArgumentsError();
        }

        auto flush_args = std::make_shared<Tuple>();
        Reference flush(FunctionContext & context) {
            if (auto object = std::get_if<Object*>(&context["this"])) {
                auto stream = dynamic_cast<std::ostream*>((*object)->c_pointer.get());

                stream->flush();

                return Reference(context.new_object());
            } else throw FunctionArgumentsError();
        }

        void setOutputStream(Context & context, Object & object) {
            Function f1 = SystemFunction{write_args, Streams::write};
            f1.extern_symbols["this"] = context.new_reference(&object);
            std::get<Object*>(object.get_property("write", context))->functions.push_front(f1);

            Function f2 = SystemFunction{flush_args, Streams::flush};
            f2.extern_symbols["this"] = context.new_reference(&object);
            std::get<Object*>(object.get_property("flush", context))->functions.push_front(f2);
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
            context.get_function("scan").push_front(SystemFunction{scan_args, scan});
            context.get_function("InputFile").push_front(SystemFunction{input_file_args, input_file});
            context.get_function("OutputFile").push_front(SystemFunction{output_file_args, output_file});


            auto console = std::get<Object*>(context["Console"]);

            auto in = context.new_object();
            setInputStream(context, *in);
            in->c_pointer = &std::cin;
            console->properties["in"] = in;

            auto out = context.new_object();
            setOutputStream(context, *out);
            out->c_pointer = &std::cout;
            console->properties["out"] = out;

            auto err = context.new_object();
            setOutputStream(context, *err);
            err->c_pointer = &std::cerr;
            console->properties["err"] = err;
        }

    }

}
