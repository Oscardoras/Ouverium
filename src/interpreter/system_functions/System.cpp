#include <any>
#include <chrono>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <ouverium/types.h>

#include "SystemFunction.hpp"

#include "../Interpreter.hpp"

#include "../../parser/Expressions.hpp"


namespace Interpreter::SystemFunctions::System {

    using TCPSocket = boost::asio::ip::tcp::socket;

    using TCPAcceptor = boost::asio::ip::tcp::acceptor;

    using UDPSocket = boost::asio::ip::udp::socket;

}


namespace Interpreter::SystemFunctions {
    template<>
    inline std::optional<std::filesystem::path> get_arg<std::filesystem::path>(Data const& data) {
        if (auto opt = get_arg<std::string>(data))
            return std::filesystem::path(*opt);

        return std::nullopt;
    }

    template<>
    inline std::optional<std::reference_wrapper<std::istream>> get_arg<std::istream&>(Data const& data) {
        if (auto opt = get_arg<std::ios&>(data))
            return dynamic_cast<std::istream&>(opt->get());

        return std::nullopt;
    }

    template<>
    inline std::optional<std::reference_wrapper<std::ostream>> get_arg<std::ostream&>(Data const& data) {
        if (auto opt = get_arg<std::ios&>(data))
            return dynamic_cast<std::ostream&>(opt->get());

        return std::nullopt;
    }

    template<>
    inline std::optional<std::reference_wrapper<std::fstream>> get_arg<std::fstream&>(Data const& data) {
        if (auto opt = get_arg<std::ios&>(data))
            return dynamic_cast<std::fstream&>(opt->get());

        return std::nullopt;
    }
}

namespace Interpreter::SystemFunctions::System {

    Reference stream_has(std::ios& stream) {
        return Data(static_cast<bool>(stream));
    }

    Reference stream_read1(std::istream& stream) {
        return Data(static_cast<char>(stream.get()));
    }

    Reference stream_read2(std::istream& stream, OV_INT size) {
        std::vector<char> buffer(size);
        stream.read(buffer.data(), static_cast<long>(size));

        auto object = GC::new_object();
        object->array.reserve(size);
        for (size_t i = 0; i < size; ++i)
            object->array.emplace_back(buffer[i]);

        return Data(object);
    }

    Reference stream_get_available(std::istream& stream) {
        return Data(static_cast<OV_INT>(stream.rdbuf()->in_avail()));
    }

    Reference stream_scan(std::istream& stream) {
        std::string str;
        std::getline(stream, str);

        return Data(GC::new_object(str));
    }

    Reference stream_write1(std::ostream& stream, char byte) {
        stream.put(byte);

        return {};
    }

    Reference stream_write2(std::ostream& stream, ObjectPtr const& bytes) {
        std::vector<char> buffer(bytes->array.size());
        for (size_t i = 0; i < buffer.size(); ++i)
            buffer[i] = bytes->array[i].get<char>();
        stream.write(buffer.data(), static_cast<long>(buffer.size()));

        return {};
    }

    Reference stream_flush(std::ostream& stream) {
        stream.flush();

        return {};
    }

    Reference stream_print(FunctionContext& context, std::ostream& stream, Data const& data) {
        stream << Interpreter::string_from(context, data);

        return {};
    }


    Reference file_open(std::string const& path) {
        return Data(std::static_pointer_cast<std::ios>(std::make_shared<std::fstream>(path)));
    }

    Reference file_close(std::fstream& stream) {
        stream.close();

        return {};
    }

    Reference file_get_current_directory() {
        return Data(GC::new_object(std::filesystem::current_path().string()));
    }

    Reference file_set_current_directory(std::filesystem::path const& path) {
        std::filesystem::current_path(path);

        return {};
    }

    Reference file_exists(std::filesystem::path const& path) {
        return Data(std::filesystem::exists(path));
    }

    Reference file_size(std::filesystem::path const& path) {
        return Data(static_cast<OV_INT>(std::filesystem::file_size(path)));
    }

    Reference file_is_empty(std::filesystem::path const& path) {
        return Data(std::filesystem::is_empty(path));
    }

    Reference file_is_directory(std::filesystem::path const& path) {
        return Data(std::filesystem::is_directory(path));
    }

    Reference file_create_directories(std::filesystem::path const& path) {
        return Data(std::filesystem::create_directory(path));
    }

    Reference file_copy(std::filesystem::path const& from, std::filesystem::path const& to) {
        std::filesystem::copy(from, to);

        return {};
    }
    Reference file_rename(std::filesystem::path const& from, std::filesystem::path const& to) {
        std::filesystem::rename(from, to);

        return {};
    }

    Reference file_delete(std::filesystem::path const& path) {
        return Data(static_cast<OV_INT>(std::filesystem::remove_all(path)));
    }

    Reference file_children(std::filesystem::path const& path) {
        auto obj = GC::new_object();
        for (auto const& child : std::filesystem::directory_iterator(path))
            obj->array.emplace_back(GC::new_object(child.path().string()));

        return Data(obj);
    }

    Reference file_concatenate(std::filesystem::path const& from, std::filesystem::path const& to) {
        return Data(GC::new_object((from / to).string()));
    }

    Reference file_parent(std::filesystem::path const& path) {
        return Data(GC::new_object(path.parent_path().string()));
    }

    Reference file_absolute(std::filesystem::path const& path) {
        return Data(GC::new_object(std::filesystem::weakly_canonical(path).string()));
    }

    Reference file_root(std::filesystem::path const& path) {
        return Data(GC::new_object(path.root_path().string()));
    }

    Reference file_filename(std::filesystem::path const& path) {
        return Data(GC::new_object(path.filename().string()));
    }

    Reference file_extension(std::filesystem::path const& path) {
        return Data(GC::new_object(path.extension().string()));
    }

    Reference file_filename_without_extension(std::filesystem::path const& path) {
        return Data(GC::new_object(path.stem().string()));
    }


    boost::asio::io_context ioc;


    Reference TCPsocket_connect(std::string const& address, OV_INT port) {
        auto socket = std::make_shared<TCPSocket>(ioc);
        boost::system::error_code ec;
        socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);

        if (!ec) {
            return Data(socket);
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPsocket_receive(TCPSocket& socket, OV_INT size) {
        boost::system::error_code ec;
        std::vector<char> buffer(size);
        auto received = socket.receive(boost::asio::buffer(buffer), {}, ec);

        if (!ec) {
            auto object = GC::new_object();
            object->array.reserve(received);
            for (size_t i = 0; i < received; ++i)
                object->array.emplace_back(buffer[i]);
            return Data(object);
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPsocket_send(TCPSocket& socket, ObjectPtr const& data) {
        std::vector<char> buffer(data->array.size());
        for (size_t i = 0; i < data->array.size(); ++i)
            buffer[i] = data->array[i].get<char>();

        boost::system::error_code ec;
        socket.send(boost::asio::buffer(buffer), {}, ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPsocket_get_blocking(TCPSocket& socket) {
        return Data(!socket.non_blocking());
    }

    Reference TCPsocket_set_blocking(TCPSocket& socket, bool value) {
        boost::system::error_code ec;
        socket.non_blocking(!value, ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPsocket_close(TCPSocket& socket) {
        boost::system::error_code ec;
        socket.close(ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }


    Reference TCPacceptor_bind(std::string const& address, OV_INT port) {
        auto acceptor = std::make_shared<TCPAcceptor>(ioc);
        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
        acceptor->open(endpoint.protocol(), ec);
        acceptor->bind(endpoint, ec);
        acceptor->listen();

        if (!ec) {
            return Data(acceptor);
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPacceptor_accept(TCPAcceptor& acceptor) {
        auto socket = std::make_shared<TCPSocket>(ioc);
        boost::system::error_code ec;
        acceptor.accept(*socket, ec);

        if (!ec) {
            return Data(socket);
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPacceptor_get_blocking(TCPAcceptor& acceptor) {
        return Data(!acceptor.non_blocking());
    }
    Reference TCPacceptor_set_blocking(TCPAcceptor& acceptor, bool value) {
        boost::system::error_code ec;
        acceptor.non_blocking(!value, ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference TCPacceptor_close(TCPAcceptor& acceptor) {
        boost::system::error_code ec;
        acceptor.close(ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference UDPsocket_open(OV_INT protocol) {
        auto socket = std::make_shared<UDPSocket>(ioc);
        boost::system::error_code ec;
        if (protocol == 4)
            socket->open(boost::asio::ip::udp::v4(), ec);
        else if (protocol == 6)
            socket->open(boost::asio::ip::udp::v6(), ec);
        else
            throw FunctionArgumentsError();

        if (!ec) {
            return Data(socket);
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference UDPsocket_bind(std::string const& address, OV_INT port) {
        auto socket = std::make_shared<UDPSocket>(ioc);
        boost::system::error_code ec;
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
        socket->open(endpoint.protocol(), ec);
        socket->bind(endpoint, ec);

        if (!ec) {
            return Data(socket);
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference UDPsocket_receive_from(UDPSocket& socket, OV_INT size) {
        boost::system::error_code ec;
        std::vector<char> buffer(size);
        boost::asio::ip::udp::endpoint endpoint;
        auto received = socket.receive_from(boost::asio::buffer(buffer), endpoint, {}, ec);

        if (!ec) {
            auto object = GC::new_object();
            object->array.reserve(received);
            for (size_t i = 0; i < received; ++i)
                object->array.emplace_back(buffer[i]);

            return Data(GC::new_object(
                {
                    Data(GC::new_object(
                        {
                            Data(GC::new_object(Object(endpoint.address().to_string()))),
                            Data(static_cast<OV_INT>(endpoint.port()))
                        }
                    )),
                    Data(object)
                }
            ));
        } else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    auto const UDPsocket_send_to_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("socket"),
            std::make_shared<Parser::Symbol>("data"),
            std::make_shared<Parser::Tuple>(Parser::Tuple(
                {
                    std::make_shared<Parser::Symbol>("address"),
                    std::make_shared<Parser::Symbol>("port")
                }
            ))
        }
    ));
    Reference UDPsocket_send_to(FunctionContext& context) {
        try {
            auto& socket = get_arg<UDPSocket&>(context["socket"].to_data(context)).value().get();
            auto data = get_arg<ObjectPtr>(context["data"].to_data(context)).value();
            auto address = get_arg<std::string>(context["address"].to_data(context)).value();
            auto port = get_arg<OV_INT>(context["port"].to_data(context)).value();

            std::vector<char> buffer(data->array.size());
            for (size_t i = 0; i < data->array.size(); ++i)
                buffer[i] = data->array[i].get<char>();

            boost::system::error_code ec;
            boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
            socket.send_to(boost::asio::buffer(buffer), endpoint, {}, ec);

            if (!ec)
                return {};
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference UDPsocket_get_blocking(UDPSocket& socket) {
        return Data(!socket.non_blocking());
    }

    Reference UDPsocket_set_blocking(UDPSocket& socket, bool value) {
        boost::system::error_code ec;
        socket.non_blocking(!value, ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }

    Reference UDPsocket_close(UDPSocket& socket) {
        boost::system::error_code ec;
        socket.close(ec);

        if (!ec)
            return {};
        else
            return Data(static_cast<OV_INT>(ec.value()));
    }


    Reference time() {
        return Data(static_cast<OV_INT>(std::time(nullptr)));
    }

    Reference clock_system() {
        std::chrono::duration<double> d = std::chrono::system_clock::now().time_since_epoch();
        return Data(static_cast<OV_FLOAT>(d.count()));
    }

    Reference clock_steady() {
        std::chrono::duration<double> d = std::chrono::steady_clock::now().time_since_epoch();
        return Data(static_cast<OV_FLOAT>(d.count()));
    }

    Reference thread_create(FunctionContext& context, ObjectPtr const& function) {
        auto& global = context.get_global();
        auto caller = context.caller;

        return Data(std::make_shared<std::jthread>([&global, caller, function]() {
            try {
                Interpreter::call_function(global, caller, Data(function), std::make_shared<Parser::Tuple>());
            } catch (Interpreter::Exception const& ex) {
                ex.print_stack_trace(global);
            }
        }));
    }

    Reference thread_join(std::jthread& thread) {
        thread.join();

        return {};
    }

    Reference thread_detach(std::jthread& thread) {
        thread.detach();

        return {};
    }

    Reference thread_get_id(std::jthread& thread) {
        return Data(static_cast<OV_INT>(std::hash<std::thread::id>{}(thread.get_id())));
    }

    Reference thread_current_id() {
        return Data(static_cast<OV_INT>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    }

    Reference thread_sleep(Data const& time) {
        try {
            std::this_thread::sleep_for(std::chrono::duration<double>(time.get<OV_INT>()));
            return {};
        } catch (Data::BadAccess const&) {
            try {
                std::this_thread::sleep_for(std::chrono::duration<double>(time.get<OV_FLOAT>()));
                return {};
            } catch (Data::BadAccess const&) {
                throw FunctionArgumentsError();
            }
        }
    }

    Reference thread_hardware_concurrency() {
        return Data(static_cast<OV_INT>(std::thread::hardware_concurrency()));
    }


    Reference mutex_create() {
        return Data(std::make_shared<std::mutex>());
    }

    Reference mutex_lock(std::mutex& mutex) {
        mutex.lock();

        return {};
    }

    Reference mutex_try_lock(std::mutex& mutex) {
        return Data(mutex.try_lock());
    }

    Reference mutex_unlock(std::mutex& mutex) {
        mutex.unlock();

        return {};
    }


    Reference GC_collect() {
        GC::collect();

        return {};
    }


    void init(GlobalContext& context) {
        auto& s = context.get_global().system;
        s = GC::new_object();

        get_object(s.get_property("async"));

        add_function(s.get_property("stream_is"), check_type<std::ios&>);
        add_function(s.get_property("stream_has"), stream_has);
        add_function(s.get_property("istream_is"), check_type<std::istream&>);
        add_function(s.get_property("stream_read"), stream_read1);
        add_function(s.get_property("stream_read"), stream_read2);
        add_function(s.get_property("stream_get_available"), stream_get_available);
        add_function(s.get_property("stream_scan"), stream_scan);
        add_function(s.get_property("ostream_is"), check_type<std::ostream&>);
        add_function(s.get_property("stream_write"), stream_write1);
        add_function(s.get_property("stream_write"), stream_write2);
        add_function(s.get_property("stream_flush"), stream_flush);
        add_function(s.get_property("stream_print"), stream_print);

        add_function(s.get_property("file_is"), check_type<std::fstream&>);
        add_function(s.get_property("file_open"), file_open);
        add_function(s.get_property("file_close"), file_close);
        add_function(s.get_property("file_get_current_directory"), file_get_current_directory);
        add_function(s.get_property("file_set_current_directory"), file_set_current_directory);
        add_function(s.get_property("file_exists"), file_exists);
        add_function(s.get_property("file_size"), file_size);
        add_function(s.get_property("file_is_empty"), file_is_empty);
        add_function(s.get_property("file_is_directory"), file_is_directory);
        add_function(s.get_property("file_create_directories"), file_create_directories);
        add_function(s.get_property("file_copy"), file_copy);
        add_function(s.get_property("file_rename"), file_rename);
        add_function(s.get_property("file_delete"), file_delete);
        add_function(s.get_property("file_children"), file_children);
        add_function(s.get_property("file_concatenate"), file_concatenate);
        add_function(s.get_property("file_parent"), file_parent);
        add_function(s.get_property("file_absolute"), file_absolute);
        add_function(s.get_property("file_root"), file_root);
        add_function(s.get_property("file_filename"), file_filename);
        add_function(s.get_property("file_extension"), file_extension);
        add_function(s.get_property("file_filename_without_extension"), file_filename_without_extension);

        add_function(s.get_property("TCPsocket_is"), check_type<TCPSocket&>);
        add_function(s.get_property("TCPsocket_connect"), TCPsocket_connect);
        add_function(s.get_property("TCPsocket_receive"), TCPsocket_receive);
        add_function(s.get_property("TCPsocket_send"), TCPsocket_send);
        add_function(s.get_property("TCPsocket_get_blocking"), TCPsocket_get_blocking);
        add_function(s.get_property("TCPsocket_set_blocking"), TCPsocket_set_blocking);
        add_function(s.get_property("TCPsocket_close"), TCPsocket_close);

        add_function(s.get_property("TCPacceptor_is"), check_type<TCPAcceptor&>);
        add_function(s.get_property("TCPacceptor_bind"), TCPacceptor_bind);
        add_function(s.get_property("TCPacceptor_accept"), TCPacceptor_accept);
        add_function(s.get_property("TCPacceptor_get_blocking"), TCPacceptor_get_blocking);
        add_function(s.get_property("TCPacceptor_set_blocking"), TCPacceptor_set_blocking);
        add_function(s.get_property("TCPacceptor_close"), TCPacceptor_close);

        add_function(s.get_property("UDPsocket_is"), check_type<UDPSocket&>);
        add_function(s.get_property("UDPsocket_bind"), UDPsocket_bind);
        add_function(s.get_property("UDPsocket_open"), UDPsocket_open);
        add_function(s.get_property("UDPsocket_receive_from"), UDPsocket_receive_from);
        add_function(s.get_property("UDPsocket_send_to"), UDPsocket_send_to_args, UDPsocket_send_to);
        add_function(s.get_property("UDPsocket_get_blocking"), UDPsocket_get_blocking);
        add_function(s.get_property("UDPsocket_set_blocking"), UDPsocket_set_blocking);
        add_function(s.get_property("UDPsocket_close"), UDPsocket_close);

        add_function(s.get_property("time"), time);
        add_function(s.get_property("clock_system"), clock_system);
        add_function(s.get_property("clock_steady"), clock_steady);

        add_function(s.get_property("thread_is"), check_type<std::jthread&>);
        add_function(s.get_property("thread_create"), thread_create);
        add_function(s.get_property("thread_join"), thread_join);
        add_function(s.get_property("thread_detach"), thread_detach);
        add_function(s.get_property("thread_get_id"), thread_get_id);
        add_function(s.get_property("thread_current_id"), thread_current_id);
        add_function(s.get_property("thread_sleep"), thread_sleep);
        add_function(s.get_property("thread_hardware_concurrency"), thread_hardware_concurrency);

        add_function(s.get_property("mutex_is"), check_type<std::mutex&>);
        add_function(s.get_property("mutex_create"), mutex_create);
        add_function(s.get_property("mutex_lock"), mutex_lock);
        add_function(s.get_property("mutex_try_lock"), mutex_try_lock);
        add_function(s.get_property("mutex_unlock"), mutex_unlock);

        add_function(s.get_property("GC_collect"), GC_collect);

        Interpreter::set(context, s.get_property("in"), Data(std::reference_wrapper<std::ios>(std::cin)));
        Interpreter::set(context, s.get_property("out"), Data(std::reference_wrapper<std::ios>(std::cout)));
        Interpreter::set(context, s.get_property("err"), Data(std::reference_wrapper<std::ios>(std::cerr)));
    }

}
