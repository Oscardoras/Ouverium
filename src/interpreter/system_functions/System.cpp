#include <ctime>
#include <fstream>
#include <iostream>

#include "System.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        auto file_path_args = std::make_shared<Parser::Symbol>("path");
        auto file_args = std::make_shared<Parser::Symbol>("file");

        Reference file_read(FunctionContext& context) {
            try {
                auto file = context["file"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::istream&>(file->c_obj.get<std::ios>());

                std::string str;
                getline(stream, str);

                return Data(context.new_object(str));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_has(FunctionContext& context) {
            try {
                auto file = context["file"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::istream&>(file->c_obj.get<std::ios>());
                return Data(static_cast<bool>(stream));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto file_write_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("file"),
                std::make_shared<Parser::Symbol>("data")
            }
        ));
        Reference file_write(FunctionContext& context) {
            try {
                auto file = context["file"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::ostream&>(file->c_obj.get<std::ios>());
                auto data = context["data"].to_data(context);

                stream << Interpreter::string_from(context, data);

                return Data(context.new_object());
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_flush(FunctionContext& context) {
            try {
                auto file = context["file"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::ostream&>(file->c_obj.get<std::ios>());

                stream.flush();

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_open(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                object->c_obj = std::shared_ptr<std::ios>(std::make_shared<std::fstream>(path));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_get_working_directory(FunctionContext& context) {
            return Data(context.new_object(std::filesystem::current_path().string()));
        }

        Reference file_set_working_directory(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();
                std::filesystem::current_path(path);

                return Data();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_exists(FunctionContext& context) {
            try {
                std::filesystem::path p = context["path"].to_data(context).get<Object*>()->to_string();
                return Data(std::filesystem::exists(p));
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_is_directory(FunctionContext& context) {
            try {
                std::filesystem::path p = context["path"].to_data(context).get<Object*>()->to_string();
                return Data(std::filesystem::is_directory(p));
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_create_directories(FunctionContext& context) {
            try {
                std::filesystem::path p = context["path"].to_data(context).get<Object*>()->to_string();
                return Data(std::filesystem::create_directory(p));
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }

        auto file_copy_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("from"),
                std::make_shared<Parser::Symbol>("to")
            }
        ));
        Reference file_copy(FunctionContext& context) {
            try {
                std::filesystem::path from = context["from"].to_data(context).get<Object*>()->to_string();
                std::filesystem::path to = context["to"].to_data(context).get<Object*>()->to_string();
                std::filesystem::copy(from, to);
                return Data{};
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_delete(FunctionContext& context) {
            try {
                std::filesystem::path p = context["path"].to_data(context).get<Object*>()->to_string();
                return Data(static_cast<INT>(std::filesystem::remove_all(p)));
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }


        Reference time(FunctionContext&) {
            return Data((INT) std::time(nullptr));
        }


        auto weak_reference_args = std::make_shared<Parser::Symbol>("object");
        Reference weak_reference(FunctionContext& context) {
            try {
                auto obj = context["object"].to_data(context).get<Object*>();

                if (!obj->weak_ref) {
                    auto reference = context.new_object();
                    reference->c_obj = WeakReference{ obj };
                    obj->weak_ref = reference;
                }

                return Data(obj->weak_ref);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto weak_reference_get_args = std::make_shared<Parser::Symbol>("reference");
        Reference weak_reference_get(FunctionContext& context) {
            try {
                auto reference = context["reference"].to_data(context).get<Object*>();

                auto weak_reference = reference->c_obj.get<WeakReference>();

                return Data(weak_reference.obj);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto GC_collect_args = std::make_shared<Parser::Tuple>();
        Reference GC_collect(FunctionContext& context) {
            context.GC_collect();
            return Reference();
        }


        void init(GlobalContext& context) {
            auto& s = *context.get_global().system;
            
            s["file_read"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_args, file_read });
            s["file_has"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_args, file_has });
            s["file_write"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_write_args, file_write });
            s["file_flush"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_args, file_flush });
            s["file_open"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_open });
            s["file_get_working_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), file_get_working_directory });
            s["file_set_working_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_set_working_directory });
            s["file_exists"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_exists });
            s["file_is_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_is_directory });
            s["file_create_directories"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_create_directories });
            s["file_copy"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_copy_args, file_copy });
            s["file_delete"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_delete });

            s["time"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), time });

            s["weak_reference"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ weak_reference_args, weak_reference });
            s["weak_reference_get"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ weak_reference_get_args, weak_reference_get });
            s["GC_collect"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ GC_collect_args, GC_collect });


            auto system = context["System"].to_data(context).get<Object*>();

            auto in = context.new_object();
            in->c_obj = std::reference_wrapper<std::ios>(std::cin);
            system->properties["in"] = in;

            auto out = context.new_object();
            out->c_obj = std::reference_wrapper<std::ios>(std::cout);
            system->properties["out"] = out;

            auto err = context.new_object();
            err->c_obj = std::reference_wrapper<std::ios>(std::cerr);
            system->properties["err"] = err;
        }

    }

}
