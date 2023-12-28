#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Streams.hpp"


namespace Interpreter {

    namespace Streams {

        auto print_args = std::make_shared<Parser::Symbol>("data");
        Reference print(FunctionContext& context) {
            auto data = context["data"];

            auto str = Interpreter::string_from(context, data);
            if (!str.empty())
                std::cout << str << std::endl;

            return Reference();
        }

        auto scan_args = std::make_shared<Parser::Tuple>();
        Reference scan(FunctionContext& context) {
            std::string str;
            getline(std::cin, str);

            return Data(context.new_object(str));
        }

        auto read_args = std::make_shared<Parser::Tuple>();
        Reference read(FunctionContext& context) {
            try {
                auto object = context["this"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::istream&>(object->c_obj.get<std::ios>());

                std::string str;
                getline(stream, str);

                return Data(context.new_object(str));
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        auto has_args = std::make_shared<Parser::Tuple>();
        Reference has(FunctionContext& context) {
            try {
                auto object = context["this"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::istream&>(object->c_obj.get<std::ios>());
                return Reference(Data(static_cast<bool>(stream)));
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        void setInputStream(Context& context, Object& object) {
            Function f1 = SystemFunction{ read_args, Streams::read };
            f1.extern_symbols.emplace("this", context.new_reference(&object));
            object["read"].to_data(context).get<Object*>()->functions.push_front(f1);

            Function f2 = SystemFunction{ has_args, Streams::has };
            f2.extern_symbols.emplace("this", context.new_reference(&object));
            object["has"].to_data(context).get<Object*>()->functions.push_front(f2);
        }

        auto write_args = std::make_shared<Parser::Symbol>("data");
        Reference write(FunctionContext& context) {
            try {
                auto object = context["this"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::ostream&>(object->c_obj.get<std::ios>());
                auto data = context["data"].to_data(context);

                stream << Interpreter::string_from(context, data);

                return Reference(Data(context.new_object()));
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        auto flush_args = std::make_shared<Parser::Tuple>();
        Reference flush(FunctionContext& context) {
            try {
                auto object = context["this"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::ostream&>(object->c_obj.get<std::ios>());

                stream.flush();

                return Reference();
            } catch (Data::BadAccess& e) {
                throw FunctionArgumentsError();
            }
        }

        void setOutputStream(Context& context, Object& object) {
            Function f1 = SystemFunction{ write_args, Streams::write };
            f1.extern_symbols.emplace("this", context.new_reference(&object));
            object["write"].to_data(context).get<Object*>()->functions.push_front(f1);

            Function f2 = SystemFunction{ flush_args, Streams::flush };
            f2.extern_symbols.emplace("this", context.new_reference(&object));
            object["flush"].to_data(context).get<Object*>()->functions.push_front(f2);
        }

        auto input_file_args = std::make_shared<Parser::Symbol>("path");
        Reference input_file(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                setInputStream(context, *object);
                object->c_obj = std::shared_ptr<std::ios>(std::make_shared<std::ifstream>(path));

                return Reference(Data(object));
            } catch (Data::BadAccess& e) {
                throw Interpreter::FunctionArgumentsError();
            } catch (std::exception& e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        auto output_file_args = std::make_shared<Parser::Symbol>("path");
        Reference output_file(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                setOutputStream(context, *object);
                object->c_obj = std::shared_ptr<std::ios>(std::make_shared<std::ofstream>(path));

                return Reference(object);
            } catch (Data::BadAccess& e) {
                throw Interpreter::FunctionArgumentsError();
            } catch (std::exception& e) {
                throw Interpreter::FunctionArgumentsError();
            }
        }

        Reference wd(FunctionContext& context) {
            std::filesystem::path p(".");
            return Data(context.new_object(std::filesystem::canonical(p).string()));
        }

        void init(GlobalContext& context) {
            context.get_function("print").push_front(SystemFunction{ print_args, print });
            context.get_function("scan").push_front(SystemFunction{ scan_args, scan });
            context.get_function("InputFile").push_front(SystemFunction{ input_file_args, input_file });
            context.get_function("OutputFile").push_front(SystemFunction{ output_file_args, output_file });

            context.get_function("wd").push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), wd });


            auto console = context["Console"].to_data(context).get<Object*>();

            auto in = context.new_object();
            setInputStream(context, *in);
            in->c_obj = std::reference_wrapper<std::ios>(std::cin);
            console->properties["in"] = in;

            auto out = context.new_object();
            setOutputStream(context, *out);
            out->c_obj = std::reference_wrapper<std::ios>(std::cout);
            console->properties["out"] = out;

            auto err = context.new_object();
            setOutputStream(context, *err);
            err->c_obj = std::reference_wrapper<std::ios>(std::cerr);
            console->properties["err"] = err;
        }

    }

}
