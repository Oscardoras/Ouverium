#include <condition_variable>
#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

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


        auto time_args = std::make_shared<Parser::Tuple>();
        Reference time(FunctionContext&) {
            return Data(static_cast<INT>(std::time(nullptr)));
        }

        auto clock_system_args = std::make_shared<Parser::Tuple>();
        Reference clock_system(FunctionContext&) {
            std::chrono::duration<double> d = std::chrono::system_clock::now().time_since_epoch();
            return Data(static_cast<FLOAT>(d.count()));
        }

        auto clock_steady_args = std::make_shared<Parser::Tuple>();
        Reference clock_steady(FunctionContext&) {
            std::chrono::duration<double> d = std::chrono::steady_clock::now().time_since_epoch();
            return Data(static_cast<FLOAT>(d.count()));
        }


        class Thread : protected std::thread {
        protected:

            bool joined_detached = false;

        public:
            using std::thread::thread;

            void join() {
                if (!joined_detached) {
                    std::thread::join();
                    joined_detached = true;
                } else {
                    throw FunctionArgumentsError();
                }
            }

            void detach() {
                if (!joined_detached) {
                    std::thread::detach();
                    joined_detached = true;
                } else {
                    throw FunctionArgumentsError();
                }
            }

            INT get_id() const {
                return static_cast<INT>(std::hash<std::thread::id>{}(std::thread::get_id()));
            }

            ~Thread() {
                if (!joined_detached) {
                    std::thread::detach();
                }
            }
        };

        auto thread_is_args = std::make_shared<Parser::Symbol>("thread");
        Reference thread_is(FunctionContext& context) {
            try {
                context["thread"].to_data(context).get<Object*>()->c_obj.get<Thread>();

                return Data(true);
            } catch (std::bad_any_cast const&) {
                return Data(false);
            } catch (Data::BadAccess const&) {
                return Data(false);
            }
        }

        auto thread_create_args = std::make_shared<Parser::Symbol>("function");
        Reference thread_create(FunctionContext& context) {
            try {
                auto function = context["function"].to_data(context).get<Object*>();

                auto& global = context.get_global();
                auto expression = context.expression;

                auto obj = context.new_object();
                obj->c_obj = std::make_shared<Thread>([&global, expression, function]() {
                    try {
                        Interpreter::call_function(global, expression, Data(function), std::make_shared<Parser::Tuple>());
                    } catch (Interpreter::Exception const& ex) {
                        ex.print_stack_trace(global);
                    }
                });
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto thread_join_args = std::make_shared<Parser::Symbol>("thread");
        Reference thread_join(FunctionContext& context) {
            try {
                auto& thread = context["thread"].to_data(context).get<Object*>()->c_obj.get<Thread>();
                thread.join();

                return Data{};
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto thread_detach_args = std::make_shared<Parser::Symbol>("thread");
        Reference thread_detach(FunctionContext& context) {
            try {
                auto& thread = context["thread"].to_data(context).get<Object*>()->c_obj.get<Thread>();
                thread.detach();

                return Data{};
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto thread_get_id_args = std::make_shared<Parser::Symbol>("thread");
        Reference thread_get_id(FunctionContext& context) {
            try {
                auto& thread = context["thread"].to_data(context).get<Object*>()->c_obj.get<Thread>();

                return Data(static_cast<INT>(thread.get_id()));
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto thread_current_id_args = std::make_shared<Parser::Tuple>();
        Reference thread_current_id(FunctionContext&) {
            return Data(static_cast<INT>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
        }

        auto thread_sleep_args = std::make_shared<Parser::Symbol>("time");
        Reference thread_sleep(FunctionContext& context) {
            auto time = context["time"].to_data(context);

            try {
                std::this_thread::sleep_for(std::chrono::duration<double>(time.get<INT>()));
                return Data{};
            } catch (Data::BadAccess const&) {
                try {
                    std::this_thread::sleep_for(std::chrono::duration<double>(time.get<FLOAT>()));
                    return Data{};
                } catch (Data::BadAccess const&) {
                    throw FunctionArgumentsError();
                }
            }
        }

        auto thread_hardware_concurrency_args = std::make_shared<Parser::Tuple>();
        Reference thread_hardware_concurrency(FunctionContext&) {
            return Data(static_cast<INT>(std::thread::hardware_concurrency()));
        }


        auto mutex_is_args = std::make_shared<Parser::Symbol>("mutex");
        Reference mutex_is(FunctionContext& context) {
            try {
                context["mutex"].to_data(context).get<Object*>()->c_obj.get<std::mutex>();

                return Data(true);
            } catch (std::bad_any_cast const&) {
                return Data(false);
            } catch (Data::BadAccess const&) {
                return Data(false);
            }
        }

        auto mutex_create_args = std::make_shared<Parser::Tuple>();
        Reference mutex_create(FunctionContext& context) {
            try {
                auto obj = context.new_object();
                obj->c_obj = std::make_shared<std::mutex>();
                return Data(obj);
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto mutex_lock_args = std::make_shared<Parser::Symbol>("mutex");
        Reference mutex_lock(FunctionContext& context) {
            try {
                auto& mutex = context["mutex"].to_data(context).get<Object*>()->c_obj.get<std::mutex>();
                mutex.lock();
                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto mutex_try_lock_args = std::make_shared<Parser::Symbol>("mutex");
        Reference mutex_try_lock(FunctionContext& context) {
            try {
                auto& mutex = context["mutex"].to_data(context).get<Object*>()->c_obj.get<std::mutex>();
                return Data(mutex.try_lock());
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto mutex_unlock_args = std::make_shared<Parser::Symbol>("mutex");
        Reference mutex_unlock(FunctionContext& context) {
            try {
                auto& mutex = context["mutex"].to_data(context).get<Object*>()->c_obj.get<std::mutex>();
                mutex.unlock();
                return Data{};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
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

            s["time"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ time_args, time });
            s["clock_system"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ clock_system_args, clock_system });
            s["clock_steady"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ clock_steady_args, clock_steady });

            s["thread_is"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_is_args, thread_is });
            s["thread_create"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_create_args, thread_create });
            s["thread_join"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_join_args, thread_join });
            s["thread_detach"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_detach_args, thread_detach });
            s["thread_get_id"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_get_id_args, thread_get_id });
            s["thread_current_id"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_current_id_args, thread_current_id });
            s["thread_sleep"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_sleep_args, thread_sleep });
            s["thread_hardware_concurrency"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ thread_hardware_concurrency_args, thread_hardware_concurrency });

            s["mutex_is"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ mutex_is_args, mutex_is });
            s["mutex_create"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ mutex_create_args, mutex_create });
            s["mutex_lock"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ mutex_lock_args, mutex_lock });
            s["mutex_try_lock"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ mutex_try_lock_args, mutex_try_lock });
            s["mutex_unlock"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ mutex_unlock_args, mutex_unlock });

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
