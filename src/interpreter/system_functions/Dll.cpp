#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Dll.hpp"


namespace Interpreter {

    namespace Dll {

        auto include_args = std::make_shared<Parser::Symbol>("path");
        Reference include(FunctionContext & context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                std::ostringstream oss;
                oss << std::ifstream(path).rdbuf();
                std::string code = oss.str();



                return Reference();
            } catch (Data::BadAccess & e) {
                throw Interpreter::FunctionArgumentsError();                        
            } catch (std::exception & e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        void init(GlobalContext & context) {
            context.get_function("print").push_front(SystemFunction{print_args, print});
            context.get_function("scan").push_front(SystemFunction{scan_args, scan});
            context.get_function("InputFile").push_front(SystemFunction{input_file_args, input_file});
            context.get_function("OutputFile").push_front(SystemFunction{output_file_args, output_file});

            context.get_function("wd").push_front(SystemFunction{std::make_shared<Parser::Tuple>(), wd});


            auto console = context["Console"].to_data(context).get<Object*>();

            auto in = context.new_object();
            setInputStream(context, *in);
            in->stream = std::cin;
            console->properties["in"] = in;

            auto out = context.new_object();
            setOutputStream(context, *out);
            out->stream = std::cout;
            console->properties["out"] = out;

            auto err = context.new_object();
            setOutputStream(context, *err);
            err->stream = std::cerr;
            console->properties["err"] = err;
        }

    }

}
