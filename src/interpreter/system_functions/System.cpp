#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>

#include "System.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        class StdInputStream : public InputStream {

            std::reference_wrapper<std::istream> is;

        public:

            StdInputStream(std::istream& is) :
                is{ is } {}

            int read() override {
                return is.get().get();
            }

            std::string scan() override {
                std::string str;
                getline(is.get(), str);
                return str;
            }

            bool has() override {
                return static_cast<bool>(is.get());
            }

        };

        class StdOutputStream : public OutputStream {

            std::reference_wrapper<std::ostream> os;

        public:

            StdOutputStream(std::ostream& os) :
                os{ os } {}

            void write(int b) override {
                os.get().put(b);
            }

            void print(std::string const& str) override {
                os.get() << str;
            }

            void flush() override {
                os.get().flush();
            }

        };

        auto stream_is_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_is(FunctionContext& context) {
            try {
                context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>();

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
            }
        }

        auto stream_read_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_read(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<InputStream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>());

                return Data(stream.read());
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_scan_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_scan(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<InputStream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>());

                return Data(context.new_object(stream.scan()));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_has_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_has(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<InputStream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>());

                return Data(stream.has());
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_write_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("stream"),
                std::make_shared<Parser::Symbol>("byte")
            }
        ));
        Reference stream_write(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<OutputStream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>());
                auto byte = context["byte"].to_data(context).get<OV_INT>();

                stream.write(byte);

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_print_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("stream"),
                std::make_shared<Parser::Symbol>("data")
            }
        ));
        Reference stream_print(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<OutputStream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>());
                auto data = context["data"].to_data(context);

                stream.print(Interpreter::string_from(context, data));

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_flush_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_flush(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<OutputStream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<Stream>());

                stream.flush();

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }


        class FileStream : private std::shared_ptr<std::fstream>, public StdInputStream, public StdOutputStream {
        public:

            FileStream(std::string const& path) :
                std::shared_ptr<std::fstream>{ std::make_shared<std::fstream>(path) }, StdInputStream{ *this }, StdOutputStream{ *this } {}

            void close() {
                this->close();
            }

        };

        auto file_path_args = std::make_shared<Parser::Symbol>("path");
        Reference file_open(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                object->c_obj = static_cast<Stream>(FileStream(path));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto file_close_args = std::make_shared<Parser::Symbol>("file");
        Reference file_close(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<FileStream&>(context["file"].to_data(context).get<Object*>()->c_obj.get<Stream>());

                stream.close();

                return Reference();
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
                return Reference();
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }

        Reference file_delete(FunctionContext& context) {
            try {
                std::filesystem::path p = context["path"].to_data(context).get<Object*>()->to_string();
                return Data(static_cast<OV_INT>(std::filesystem::remove_all(p)));
            } catch (std::filesystem::filesystem_error const&) {
                throw FunctionArgumentsError();
            }
        }


        auto socket_open_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("address"),
                std::make_shared<Parser::Symbol>("port")
            }
        ));
        Reference socket_open(FunctionContext& context) {
            try {
                auto address = context["address"].to_data(context).get<Object*>()->to_string();
                auto port = context["port"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                object->c_obj = std::static_pointer_cast<std::ios>(std::make_shared<boost::asio::ip::tcp::iostream>(address, port));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto socket_close_args = std::make_shared<Parser::Symbol>("socket");
        Reference socket_close(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<boost::asio::ip::tcp::iostream&>(context["socket"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                stream.close();

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto acceptor_open_args = std::make_shared<Parser::Symbol>("port");
        Reference acceptor_open(FunctionContext& context) {
            try {
                auto port = context["port"].to_data(context).get<OV_INT>();

                auto object = context.new_object();
                object->c_obj = std::make_shared<boost::asio::ip::tcp::acceptor>(context.get_global().ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto acceptor_accept_args = std::make_shared<Parser::Symbol>("acceptor");
        Reference acceptor_accept(FunctionContext& context) {
            try {
                auto& acceptor = context["acceptor"].to_data(context).get<Object*>()->c_obj.get<boost::asio::ip::tcp::acceptor>();

                auto stream = std::make_shared<boost::asio::ip::tcp::iostream>();
                auto object = context.new_object();
                object->c_obj = std::static_pointer_cast<std::ios>(stream);
                acceptor.accept(stream->socket());

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }


        auto acceptor_close_args = std::make_shared<Parser::Symbol>("acceptor");
        Reference acceptor_close(FunctionContext& context) {
            try {
                auto& acceptor = context["acceptor"].to_data(context).get<Object*>()->c_obj.get<boost::asio::ip::tcp::acceptor>();

                acceptor.close();

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto time_args = std::make_shared<Parser::Tuple>();
        Reference time(FunctionContext&) {
            return Data(static_cast<OV_INT>(std::time(nullptr)));
        }

        auto clock_system_args = std::make_shared<Parser::Tuple>();
        Reference clock_system(FunctionContext&) {
            std::chrono::duration<double> d = std::chrono::system_clock::now().time_since_epoch();
            return Data(static_cast<OV_FLOAT>(d.count()));
        }

        auto clock_steady_args = std::make_shared<Parser::Tuple>();
        Reference clock_steady(FunctionContext&) {
            std::chrono::duration<double> d = std::chrono::steady_clock::now().time_since_epoch();
            return Data(static_cast<OV_FLOAT>(d.count()));
        }


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

                return Reference();
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

                return Reference();
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

                return Data(static_cast<OV_INT>(thread.get_id()));
            } catch (std::bad_any_cast const&) {
                throw FunctionArgumentsError();
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }

        auto thread_current_id_args = std::make_shared<Parser::Tuple>();
        Reference thread_current_id(FunctionContext&) {
            return Data(static_cast<OV_INT>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
        }

        auto thread_sleep_args = std::make_shared<Parser::Symbol>("time");
        Reference thread_sleep(FunctionContext& context) {
            auto time = context["time"].to_data(context);

            try {
                std::this_thread::sleep_for(std::chrono::duration<double>(time.get<OV_INT>()));
                return Reference();
            } catch (Data::BadAccess const&) {
                try {
                    std::this_thread::sleep_for(std::chrono::duration<double>(time.get<OV_FLOAT>()));
                    return Reference();
                } catch (Data::BadAccess const&) {
                    throw FunctionArgumentsError();
                }
            }
        }

        auto thread_hardware_concurrency_args = std::make_shared<Parser::Tuple>();
        Reference thread_hardware_concurrency(FunctionContext&) {
            return Data(static_cast<OV_INT>(std::thread::hardware_concurrency()));
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
                return Reference();
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
                return Reference();
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

            s["stream_is"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_is_args, stream_is });
            s["stream_read"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_read_args, stream_read });
            s["stream_scan"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_scan_args, stream_scan });
            s["stream_has"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_has_args, stream_has });
            s["stream_write"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_write_args, stream_write });
            s["stream_print"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_print_args, stream_print });
            s["stream_flush"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ stream_flush_args, stream_flush });

            s["file_open"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_open });
            s["file_close"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_close_args, file_close });
            s["file_get_working_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), file_get_working_directory });
            s["file_set_working_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_set_working_directory });
            s["file_exists"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_exists });
            s["file_is_directory"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_is_directory });
            s["file_create_directories"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_create_directories });
            s["file_copy"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_copy_args, file_copy });
            s["file_delete"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ file_path_args, file_delete });

            s["socket_open"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ socket_open_args, socket_open });
            s["socket_close"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ socket_close_args, socket_close });
            s["acceptor_open"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ acceptor_open_args, acceptor_open });
            s["acceptor_accept"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ acceptor_accept_args, acceptor_accept });
            s["acceptor_close"].to_data(context).get<Object*>()->functions.push_front(SystemFunction{ acceptor_close_args, acceptor_close });

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
            in->c_obj = static_cast<Stream>(StdInputStream(std::cin));
            system->properties["in"] = in;

            auto out = context.new_object();
            out->c_obj = static_cast<Stream>(StdOutputStream(std::cout));
            system->properties["out"] = out;

            auto err = context.new_object();
            err->c_obj = static_cast<Stream>(StdOutputStream(std::cerr));
            system->properties["err"] = err;
        }

    }

}
