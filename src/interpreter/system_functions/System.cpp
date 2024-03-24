#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        auto stream_is_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_is(FunctionContext& context) {
            try {
                context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>();

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
            }
        }

        auto stream_has_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_has(FunctionContext& context) {
            try {
                auto& stream = context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>();

                return Data(static_cast<bool>(stream));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto istream_is_args = std::make_shared<Parser::Symbol>("stream");
        Reference istream_is(FunctionContext& context) {
            try {
                dynamic_cast<std::istream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
            }
        }

        auto stream_read1_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_read1(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(static_cast<char>(stream.get()));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_read2_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("stream"),
                std::make_shared<Parser::Symbol>("size")
            }
        ));
        Reference stream_read2(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());
                auto size = static_cast<size_t>(context["size"].to_data(context).get<OV_INT>());

                std::vector<char> buffer(size);
                stream.read(buffer.data(), size);

                auto object = context.new_object();
                object->array.reserve(size);
                for (size_t i = 0; i < size; ++i)
                    object->array.push_back(Data(buffer[i]));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_get_available_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_get_available(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(static_cast<OV_INT>(stream.rdbuf()->in_avail()));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_scan_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_scan(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                std::string str;
                std::getline(stream, str);

                return Data(context.new_object(str));
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto ostream_is_args = std::make_shared<Parser::Symbol>("stream");
        Reference ostream_is(FunctionContext& context) {
            try {
                dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
            }
        }

        auto stream_write1_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("stream"),
                std::make_shared<Parser::Symbol>("byte")
            }
        ));
        Reference stream_write1(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());
                auto byte = context["byte"].to_data(context).get<char>();

                stream.put(byte);

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_write2_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("stream"),
                std::make_shared<Parser::Symbol>("bytes")
            }
        ));
        Reference stream_write2(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());
                auto bytes = context["bytes"].to_data(context).get<Object*>();

                std::vector<char> buffer(bytes->array.size());
                for (size_t i = 0; i < buffer.size(); ++i)
                    buffer[i] = bytes->array[i].get<char>();
                stream.write(buffer.data(), buffer.size());

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto stream_flush_args = std::make_shared<Parser::Symbol>("stream");
        Reference stream_flush(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                stream.flush();

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
                auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<Object*>()->c_obj.get<std::ios>());
                auto data = context["data"].to_data(context);

                stream << Interpreter::string_from(context, data);

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }


        auto file_is_args = std::make_shared<Parser::Symbol>("file");
        Reference file_is(FunctionContext& context) {
            try {
                dynamic_cast<std::fstream&>(context["file"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
            }
        }

        auto file_path_args = std::make_shared<Parser::Symbol>("path");
        Reference file_open(FunctionContext& context) {
            try {
                auto path = context["path"].to_data(context).get<Object*>()->to_string();

                auto object = context.new_object();
                object->c_obj.set<std::ios>(std::make_unique<std::fstream>(path));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto file_close_args = std::make_shared<Parser::Symbol>("file");
        Reference file_close(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<std::fstream&>(context["file"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

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


        auto socket_is_args = std::make_shared<Parser::Symbol>("socket");
        Reference socket_is(FunctionContext& context) {
            try {
                dynamic_cast<boost::asio::ip::tcp::iostream&>(context["socket"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
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
                auto port = context["port"].to_data(context).get<OV_INT>();

                auto object = context.new_object();
                object->c_obj.set<std::ios>(std::make_unique<boost::asio::ip::tcp::iostream>(address, std::to_string(port)));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto socket_get_blocking_args = std::make_shared<Parser::Symbol>("socket");
        Reference socket_get_blocking(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<boost::asio::ip::tcp::iostream&>(context["socket"].to_data(context).get<Object*>()->c_obj.get<std::ios>());

                return Data(!stream.socket().non_blocking());
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto socket_set_blocking_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("socket"),
                std::make_shared<Parser::Symbol>("value")
            }
        ));
        Reference socket_set_blocking(FunctionContext& context) {
            try {
                auto& stream = dynamic_cast<boost::asio::ip::tcp::iostream&>(context["socket"].to_data(context).get<Object*>()->c_obj.get<std::ios>());
                auto value = context["value"].to_data(context).get<bool>();

                stream.socket().non_blocking(!value);

                return Reference();
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


        auto acceptor_is_args = std::make_shared<Parser::Symbol>("acceptor");
        Reference acceptor_is(FunctionContext& context) {
            try {
                context["acceptor"].to_data(context).get<Object*>()->c_obj.get<boost::asio::ip::tcp::acceptor>();

                return Data(true);
            } catch (std::exception const&) {
                return Data(false);
            }
        }

        auto acceptor_open_args = std::make_shared<Parser::Symbol>("port");
        Reference acceptor_open(FunctionContext& context) {
            try {
                auto port = context["port"].to_data(context).get<OV_INT>();

                auto object = context.new_object();
                boost::asio::io_context ioc;
                object->c_obj.set(std::make_unique<boost::asio::ip::tcp::acceptor>(
                    ioc,
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)
                ));

                return Data(object);
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto acceptor_get_blocking_args = std::make_shared<Parser::Symbol>("acceptor");
        Reference acceptor_get_blocking(FunctionContext& context) {
            try {
                auto& acceptor = context["acceptor"].to_data(context).get<Object*>()->c_obj.get<boost::asio::ip::tcp::acceptor>();

                return Data(!acceptor.non_blocking());
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto acceptor_set_blocking_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
            {
                std::make_shared<Parser::Symbol>("acceptor"),
                std::make_shared<Parser::Symbol>("value")
            }
        ));
        Reference acceptor_set_blocking(FunctionContext& context) {
            try {
                auto& acceptor = context["acceptor"].to_data(context).get<Object*>()->c_obj.get<boost::asio::ip::tcp::acceptor>();
                auto value = context["value"].to_data(context).get<bool>();

                acceptor.non_blocking(!value);

                return Reference();
            } catch (std::exception const&) {
                throw FunctionArgumentsError();
            }
        }

        auto acceptor_accept_args = std::make_shared<Parser::Symbol>("acceptor");
        Reference acceptor_accept(FunctionContext& context) {
            try {
                auto& acceptor = context["acceptor"].to_data(context).get<Object*>()->c_obj.get<boost::asio::ip::tcp::acceptor>();

                auto stream = std::make_unique<boost::asio::ip::tcp::iostream>();
                boost::system::error_code ec;
                acceptor.accept(stream->socket(), ec);

                if (!ec) {
                    auto object = context.new_object();
                    object->c_obj.set<std::ios>(std::move(stream));
                    return Data(object);
                } else return Reference();
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

            OV_INT get_id() const {
                return static_cast<OV_INT>(std::hash<std::thread::id>{}(std::thread::get_id()));
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
                auto caller = context.caller;

                auto obj = context.new_object();
                obj->c_obj.set(std::make_unique<Thread>([&global, caller, function]() {
                    try {
                        Interpreter::call_function(global, caller, Data(function), std::make_shared<Parser::Tuple>());
                    } catch (Interpreter::Exception const& ex) {
                        ex.print_stack_trace(global);
                    }
                }));
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
                obj->c_obj.set(std::make_unique<std::mutex>());
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


        auto GC_collect_args = std::make_shared<Parser::Tuple>();
        Reference GC_collect(FunctionContext& context) {
            context.GC_collect();
            return Reference();
        }


        void init(GlobalContext& context) {
            auto s = context.get_global().system;

            get_object(context, s->properties["stream_is"])->functions.push_front(SystemFunction{ stream_is_args, stream_is });
            get_object(context, s->properties["stream_has"])->functions.push_front(SystemFunction{ stream_has_args, stream_has });
            get_object(context, s->properties["istream_is"])->functions.push_front(SystemFunction{ istream_is_args, istream_is });
            get_object(context, s->properties["stream_read"])->functions.push_front(SystemFunction{ stream_read1_args, stream_read1 });
            get_object(context, s->properties["stream_read"])->functions.push_front(SystemFunction{ stream_read2_args, stream_read2 });
            get_object(context, s->properties["stream_get_available"])->functions.push_front(SystemFunction{ stream_get_available_args, stream_get_available });
            get_object(context, s->properties["stream_scan"])->functions.push_front(SystemFunction{ stream_scan_args, stream_scan });
            get_object(context, s->properties["ostream_is"])->functions.push_front(SystemFunction{ ostream_is_args, ostream_is });
            get_object(context, s->properties["stream_write"])->functions.push_front(SystemFunction{ stream_write1_args, stream_write1 });
            get_object(context, s->properties["stream_write"])->functions.push_front(SystemFunction{ stream_write2_args, stream_write2 });
            get_object(context, s->properties["stream_flush"])->functions.push_front(SystemFunction{ stream_flush_args, stream_flush });
            get_object(context, s->properties["stream_print"])->functions.push_front(SystemFunction{ stream_print_args, stream_print });

            get_object(context, s->properties["file_is"])->functions.push_front(SystemFunction{ file_is_args, file_is });
            get_object(context, s->properties["file_open"])->functions.push_front(SystemFunction{ file_path_args, file_open });
            get_object(context, s->properties["file_close"])->functions.push_front(SystemFunction{ file_close_args, file_close });
            get_object(context, s->properties["file_get_working_directory"])->functions.push_front(SystemFunction{ std::make_shared<Parser::Tuple>(), file_get_working_directory });
            get_object(context, s->properties["file_set_working_directory"])->functions.push_front(SystemFunction{ file_path_args, file_set_working_directory });
            get_object(context, s->properties["file_exists"])->functions.push_front(SystemFunction{ file_path_args, file_exists });
            get_object(context, s->properties["file_is_directory"])->functions.push_front(SystemFunction{ file_path_args, file_is_directory });
            get_object(context, s->properties["file_create_directories"])->functions.push_front(SystemFunction{ file_path_args, file_create_directories });
            get_object(context, s->properties["file_copy"])->functions.push_front(SystemFunction{ file_copy_args, file_copy });
            get_object(context, s->properties["file_delete"])->functions.push_front(SystemFunction{ file_path_args, file_delete });

            get_object(context, s->properties["socket_is"])->functions.push_front(SystemFunction{ socket_is_args, socket_is });
            get_object(context, s->properties["socket_get_blocking"])->functions.push_front(SystemFunction{ socket_get_blocking_args, socket_get_blocking });
            get_object(context, s->properties["socket_set_blocking"])->functions.push_front(SystemFunction{ socket_set_blocking_args, socket_set_blocking });
            get_object(context, s->properties["socket_open"])->functions.push_front(SystemFunction{ socket_open_args, socket_open });
            get_object(context, s->properties["socket_close"])->functions.push_front(SystemFunction{ socket_close_args, socket_close });

            get_object(context, s->properties["acceptor_is"])->functions.push_front(SystemFunction{ acceptor_is_args, acceptor_is });
            get_object(context, s->properties["acceptor_get_blocking"])->functions.push_front(SystemFunction{ acceptor_get_blocking_args, acceptor_get_blocking });
            get_object(context, s->properties["acceptor_set_blocking"])->functions.push_front(SystemFunction{ acceptor_set_blocking_args, acceptor_set_blocking });
            get_object(context, s->properties["acceptor_open"])->functions.push_front(SystemFunction{ acceptor_open_args, acceptor_open });
            get_object(context, s->properties["acceptor_accept"])->functions.push_front(SystemFunction{ acceptor_accept_args, acceptor_accept });
            get_object(context, s->properties["acceptor_close"])->functions.push_front(SystemFunction{ acceptor_close_args, acceptor_close });

            get_object(context, s->properties["time"])->functions.push_front(SystemFunction{ time_args, time });
            get_object(context, s->properties["clock_system"])->functions.push_front(SystemFunction{ clock_system_args, clock_system });
            get_object(context, s->properties["clock_steady"])->functions.push_front(SystemFunction{ clock_steady_args, clock_steady });

            get_object(context, s->properties["thread_is"])->functions.push_front(SystemFunction{ thread_is_args, thread_is });
            get_object(context, s->properties["thread_create"])->functions.push_front(SystemFunction{ thread_create_args, thread_create });
            get_object(context, s->properties["thread_join"])->functions.push_front(SystemFunction{ thread_join_args, thread_join });
            get_object(context, s->properties["thread_detach"])->functions.push_front(SystemFunction{ thread_detach_args, thread_detach });
            get_object(context, s->properties["thread_get_id"])->functions.push_front(SystemFunction{ thread_get_id_args, thread_get_id });
            get_object(context, s->properties["thread_current_id"])->functions.push_front(SystemFunction{ thread_current_id_args, thread_current_id });
            get_object(context, s->properties["thread_sleep"])->functions.push_front(SystemFunction{ thread_sleep_args, thread_sleep });
            get_object(context, s->properties["thread_hardware_concurrency"])->functions.push_front(SystemFunction{ thread_hardware_concurrency_args, thread_hardware_concurrency });

            get_object(context, s->properties["mutex_is"])->functions.push_front(SystemFunction{ mutex_is_args, mutex_is });
            get_object(context, s->properties["mutex_create"])->functions.push_front(SystemFunction{ mutex_create_args, mutex_create });
            get_object(context, s->properties["mutex_lock"])->functions.push_front(SystemFunction{ mutex_lock_args, mutex_lock });
            get_object(context, s->properties["mutex_try_lock"])->functions.push_front(SystemFunction{ mutex_try_lock_args, mutex_try_lock });
            get_object(context, s->properties["mutex_unlock"])->functions.push_front(SystemFunction{ mutex_unlock_args, mutex_unlock });

            get_object(context, s->properties["GC_collect"])->functions.push_front(SystemFunction{ GC_collect_args, GC_collect });

            get_object(context, s->properties["in"])->c_obj.set(std::reference_wrapper<std::ios>(std::cin));
            get_object(context, s->properties["out"])->c_obj.set(std::reference_wrapper<std::ios>(std::cout));
            get_object(context, s->properties["err"])->c_obj.set(std::reference_wrapper<std::ios>(std::cerr));
        }

    }

}
