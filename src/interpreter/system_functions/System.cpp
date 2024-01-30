#include <ctime>
#include <fstream>
#include <iostream>

#include "System.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        auto read_args = std::make_shared<Parser::Symbol>("file");
        Reference read(FunctionContext& context) {
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

        auto has_args = std::make_shared<Parser::Symbol>("file");
        Reference has(FunctionContext& context) {
            try {
                auto file = context["file"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::istream&>(file->c_obj.get<std::ios>());
                return Data(static_cast<bool>(stream));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto write_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("file"),
                std::make_shared<Parser::Symbol>("data")
            }
        ));
        Reference write(FunctionContext& context) {
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

        auto flush_args = std::make_shared<Parser::Symbol>("file");
        Reference flush(FunctionContext& context) {
            try {
                auto file = context["file"].to_data(context).get<Object*>();
                auto& stream = dynamic_cast<std::ostream&>(file->c_obj.get<std::ios>());

                stream.flush();

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto open_args = std::make_shared<Parser::Symbol>("path");
        Reference open(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                object->c_obj = std::shared_ptr<std::ios>(std::make_shared<std::fstream>(path));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto working_directory_args = std::make_shared<Parser::Tuple>();
        Reference working_directory(FunctionContext& context) {
            std::filesystem::path p(".");
            return Data(context.new_object(std::filesystem::canonical(p).string()));
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
            (*context.get_global().system)["file_read"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ read_args, read });
            (*context.get_global().system)["file_has"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ has_args, has });
            (*context.get_global().system)["file_write"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ write_args, write });
            (*context.get_global().system)["file_flush"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ flush_args, flush });
            (*context.get_global().system)["file_open"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ open_args, open });
            (*context.get_global().system)["working_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ working_directory_args, working_directory });

            (*context.get_global().system)["time"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), time });

            (*context.get_global().system)["weak_reference"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ weak_reference_args, weak_reference });
            (*context.get_global().system)["weak_reference_get"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ weak_reference_get_args, weak_reference_get });
            (*context.get_global().system)["GC_collect"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ GC_collect_args, GC_collect });


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
