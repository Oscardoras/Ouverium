#include <any>
#include <chrono>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include <ouverium/types.h>

#include "SystemFunction.hpp"

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Interpreter::SystemFunctions::System {

    auto const stream_is_args = std::make_shared<Parser::Symbol>("stream");
    Reference stream_is(FunctionContext& context) {
        try {
            context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>();

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const stream_has_args = std::make_shared<Parser::Symbol>("stream");
    Reference stream_has(FunctionContext& context) {
        try {
            auto& stream = context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>();

            return Data(static_cast<bool>(stream));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const istream_is_args = std::make_shared<Parser::Symbol>("stream");
    Reference istream_is(FunctionContext& context) {
        try {
            dynamic_cast<std::istream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const stream_read1_args = std::make_shared<Parser::Symbol>("stream");
    Reference stream_read1(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            return Data(static_cast<char>(stream.get()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const stream_read2_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("stream"),
            std::make_shared<Parser::Symbol>("size")
        }
    ));
    Reference stream_read2(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());
            auto size = static_cast<size_t>(context["size"].to_data(context).get<OV_INT>());

            std::vector<char> buffer(size);
            stream.read(buffer.data(), static_cast<long>(size));

            auto object = GC::new_object();
            object->array.reserve(size);
            for (size_t i = 0; i < size; ++i)
                object->array.emplace_back(buffer[i]);

            return Data(object);
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const stream_get_available_args = std::make_shared<Parser::Symbol>("stream");
    Reference stream_get_available(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            return Data(static_cast<OV_INT>(stream.rdbuf()->in_avail()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const stream_scan_args = std::make_shared<Parser::Symbol>("stream");
    Reference stream_scan(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::istream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            std::string str;
            std::getline(stream, str);

            return Data(GC::new_object(str));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const ostream_is_args = std::make_shared<Parser::Symbol>("stream");
    Reference ostream_is(FunctionContext& context) {
        try {
            dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const stream_write1_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("stream"),
            std::make_shared<Parser::Symbol>("byte")
        }
    ));
    Reference stream_write1(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());
            auto byte = context["byte"].to_data(context).get<char>();

            stream.put(byte);

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const stream_write2_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("stream"),
            std::make_shared<Parser::Symbol>("bytes")
        }
    ));
    Reference stream_write2(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());
            auto bytes = context["bytes"].to_data(context).get<ObjectPtr>();

            std::vector<char> buffer(bytes->array.size());
            for (size_t i = 0; i < buffer.size(); ++i)
                buffer[i] = bytes->array[i].get<char>();
            stream.write(buffer.data(), static_cast<long>(buffer.size()));

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const stream_flush_args = std::make_shared<Parser::Symbol>("stream");
    Reference stream_flush(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            stream.flush();

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const stream_print_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("stream"),
            std::make_shared<Parser::Symbol>("data")
        }
    ));
    Reference stream_print(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::ostream&>(context["stream"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());
            auto data = context["data"].to_data(context);

            stream << Interpreter::string_from(context, data);

            return Reference();
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }


    auto const file_is_args = std::make_shared<Parser::Symbol>("file");
    Reference file_is(FunctionContext& context) {
        try {
            dynamic_cast<std::fstream&>(context["file"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const file_path_args = std::make_shared<Parser::Symbol>("path");
    Reference file_open(FunctionContext& context) {
        try {
            auto path = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            auto object = GC::new_object();
            object->c_obj.set<std::ios>(std::make_unique<std::fstream>(path));

            return Data(object);
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const file_close_args = std::make_shared<Parser::Symbol>("file");
    Reference file_close(FunctionContext& context) {
        try {
            auto& stream = dynamic_cast<std::fstream&>(context["file"].to_data(context).get<ObjectPtr>()->c_obj.get<std::ios>());

            stream.close();

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_get_current_directory(FunctionContext& /*context*/) {
        try {
            return Data(GC::new_object(std::filesystem::current_path().string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_set_current_directory(FunctionContext& context) {
        try {
            auto path = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            std::filesystem::current_path(path);

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_exists(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(std::filesystem::exists(p));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_size(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(static_cast<OV_INT>(std::filesystem::file_size(p)));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_is_empty(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(std::filesystem::is_empty(p));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_is_directory(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(std::filesystem::is_directory(p));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_create_directories(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(std::filesystem::create_directory(p));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const file_copy_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("from"),
            std::make_shared<Parser::Symbol>("to")
        }
    ));
    Reference file_copy(FunctionContext& context) {
        try {
            std::filesystem::path from = context["from"].to_data(context).get<ObjectPtr>()->to_string();
            std::filesystem::path to = context["to"].to_data(context).get<ObjectPtr>()->to_string();

            std::filesystem::copy(from, to);

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }
    Reference file_rename(FunctionContext& context) {
        try {
            std::filesystem::path from = context["from"].to_data(context).get<ObjectPtr>()->to_string();
            std::filesystem::path to = context["to"].to_data(context).get<ObjectPtr>()->to_string();

            std::filesystem::rename(from, to);

            return {};
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_delete(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(static_cast<OV_INT>(std::filesystem::remove_all(p)));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_children(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();

            auto obj = GC::new_object();
            for (auto const& child : std::filesystem::directory_iterator(p))
                obj->array.emplace_back(GC::new_object(child.path().string()));

            return Data(obj);
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_concatenate(FunctionContext& context) {
        try {
            std::filesystem::path from = context["from"].to_data(context).get<ObjectPtr>()->to_string();
            std::filesystem::path to = context["to"].to_data(context).get<ObjectPtr>()->to_string();

            return Data(GC::new_object((from / to).string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_parent(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();
            return Data(GC::new_object(p.parent_path().string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_absolute(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();
            return Data(GC::new_object(std::filesystem::weakly_canonical(p).string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_root(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();
            return Data(GC::new_object(p.root_path().string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_filename(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();
            return Data(GC::new_object(p.filename().string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_extension(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();
            return Data(GC::new_object(p.extension().string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    Reference file_filename_without_extension(FunctionContext& context) {
        try {
            std::filesystem::path p = context["path"].to_data(context).get<ObjectPtr>()->to_string();
            return Data(GC::new_object(p.stem().string()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }


    boost::asio::io_context ioc;


    using TCPSocket = boost::asio::ip::tcp::socket;

    auto const TCPsocket_is_args = std::make_shared<Parser::Symbol>("socket");
    Reference TCPsocket_is(FunctionContext& context) {
        try {
            context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPSocket>();

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const TCPsocket_connect_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("address"),
            std::make_shared<Parser::Symbol>("port")
        }
    ));
    Reference TCPsocket_connect(FunctionContext& context) {
        try {
            auto address = context["address"].to_data(context).get<ObjectPtr>()->to_string();
            auto port = context["port"].to_data(context).get<OV_INT>();

            auto socket = std::make_unique<TCPSocket>(ioc);
            boost::system::error_code ec;
            socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);

            if (!ec) {
                auto object = GC::new_object();
                object->c_obj.set(std::move(socket));
                return Data(object);
            } else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPsocket_receive_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("socket"),
            std::make_shared<Parser::Symbol>("size")
        }
    ));
    Reference TCPsocket_receive(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPSocket>();
            auto size = context["size"].to_data(context).get<OV_INT>();

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
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPsocket_send_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("socket"),
            std::make_shared<Parser::Symbol>("data")
        }
    ));
    Reference TCPsocket_send(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPSocket>();
            auto data = context["data"].to_data(context).get<ObjectPtr>();

            std::vector<char> buffer(data->array.size());
            for (size_t i = 0; i < data->array.size(); ++i)
                buffer[i] = data->array[i].get<char>();

            boost::system::error_code ec;
            socket.send(boost::asio::buffer(buffer), {}, ec);

            if (!ec)
                return {};
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPsocket_get_blocking_args = std::make_shared<Parser::Symbol>("socket");
    Reference TCPsocket_get_blocking(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPSocket>();

            return Data(!socket.non_blocking());
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPsocket_set_blocking_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("socket"),
            std::make_shared<Parser::Symbol>("value")
        }
    ));
    Reference TCPsocket_set_blocking(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPSocket>();
            auto value = context["value"].to_data(context).get<bool>();

            boost::system::error_code ec;
            socket.non_blocking(!value, ec);

            if (!ec)
                return {};
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPsocket_close_args = std::make_shared<Parser::Symbol>("socket");
    Reference TCPsocket_close(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPSocket>();

            boost::system::error_code ec;
            socket.close(ec);

            if (!ec)
                return {};
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }


    using TCPAcceptor = boost::asio::ip::tcp::acceptor;

    auto const TCPacceptor_is_args = std::make_shared<Parser::Symbol>("acceptor");
    Reference TCPacceptor_is(FunctionContext& context) {
        try {
            context["acceptor"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPAcceptor>();

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const TCPacceptor_bind_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("address"),
            std::make_shared<Parser::Symbol>("port")
        }
    ));
    Reference TCPacceptor_bind(FunctionContext& context) {
        try {
            auto address = context["address"].to_data(context).get<ObjectPtr>()->to_string();
            auto port = context["port"].to_data(context).get<OV_INT>();

            auto acceptor = std::make_unique<TCPAcceptor>(ioc);
            boost::system::error_code ec;
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
            acceptor->open(endpoint.protocol(), ec);
            acceptor->bind(endpoint, ec);
            acceptor->listen();

            if (!ec) {
                auto object = GC::new_object();
                object->c_obj.set(std::move(acceptor));
                return Data(object);
            } else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPacceptor_accept_args = std::make_shared<Parser::Symbol>("acceptor");
    Reference TCPacceptor_accept(FunctionContext& context) {
        try {
            auto& acceptor = context["acceptor"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPAcceptor>();

            auto socket = std::make_unique<TCPSocket>(ioc);
            boost::system::error_code ec;
            acceptor.accept(*socket, ec);

            if (!ec) {
                auto object = GC::new_object();
                object->c_obj.set(std::move(socket));
                return Data(object);
            } else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPacceptor_get_blocking_args = std::make_shared<Parser::Symbol>("acceptor");
    Reference TCPacceptor_get_blocking(FunctionContext& context) {
        try {
            auto& acceptor = context["acceptor"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPAcceptor>();

            return Data(!acceptor.non_blocking());
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPacceptor_set_blocking_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("acceptor"),
            std::make_shared<Parser::Symbol>("value")
        }
    ));
    Reference TCPacceptor_set_blocking(FunctionContext& context) {
        try {
            auto& acceptor = context["acceptor"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPAcceptor>();
            auto value = context["value"].to_data(context).get<bool>();

            boost::system::error_code ec;
            acceptor.non_blocking(!value, ec);

            if (!ec)
                return Reference();
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const TCPacceptor_close_args = std::make_shared<Parser::Symbol>("acceptor");
    Reference TCPacceptor_close(FunctionContext& context) {
        try {
            auto& acceptor = context["acceptor"].to_data(context).get<ObjectPtr>()->c_obj.get<TCPAcceptor>();

            boost::system::error_code ec;
            acceptor.close(ec);

            if (!ec)
                return Reference();
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    using UDPSocket = boost::asio::ip::udp::socket;

    auto const UDPsocket_is_args = std::make_shared<Parser::Symbol>("socket");
    Reference UDPsocket_is(FunctionContext& context) {
        try {
            context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<UDPSocket>();

            return Data(true);
        } catch (std::exception const&) {
            return Data(false);
        }
    }

    auto const UDPsocket_open_args = std::make_shared<Parser::Symbol>("protocol");
    Reference UDPsocket_open(FunctionContext& context) {
        try {
            auto protocol = context["protocol"].to_data(context).get<OV_INT>();

            auto socket = std::make_unique<UDPSocket>(ioc);
            boost::system::error_code ec;
            if (protocol == 4)
                socket->open(boost::asio::ip::udp::v4(), ec);
            else if (protocol == 6)
                socket->open(boost::asio::ip::udp::v6(), ec);
            else
                throw FunctionArgumentsError();

            if (!ec) {
                auto object = GC::new_object();
                object->c_obj.set(std::move(socket));
                return Data(object);
            } else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const UDPsocket_bind_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("address"),
            std::make_shared<Parser::Symbol>("port")
        }
    ));
    Reference UDPsocket_bind(FunctionContext& context) {
        try {
            auto address = context["address"].to_data(context).get<ObjectPtr>()->to_string();
            auto port = context["port"].to_data(context).get<OV_INT>();

            auto socket = std::make_unique<UDPSocket>(ioc);
            boost::system::error_code ec;
            boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
            socket->open(endpoint.protocol(), ec);
            socket->bind(endpoint, ec);

            if (!ec) {
                auto object = GC::new_object();
                object->c_obj.set(std::move(socket));
                return Data(object);
            } else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const UDPsocket_receive_from_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("socket"),
            std::make_shared<Parser::Symbol>("size")
        }
    ));
    Reference UDPsocket_receive_from(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<UDPSocket>();
            auto size = context["size"].to_data(context).get<OV_INT>();

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
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
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
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<UDPSocket>();
            auto data = context["data"].to_data(context).get<ObjectPtr>();
            auto address = context["address"].to_data(context).get<ObjectPtr>()->to_string();
            auto port = context["port"].to_data(context).get<OV_INT>();

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

    auto const UDPsocket_get_blocking_args = std::make_shared<Parser::Symbol>("socket");
    Reference UDPsocket_get_blocking(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<UDPSocket>();

            return Data(!socket.non_blocking());
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const UDPsocket_set_blocking_args = std::make_shared<Parser::Tuple>(Parser::Tuple(
        {
            std::make_shared<Parser::Symbol>("socket"),
            std::make_shared<Parser::Symbol>("value")
        }
    ));
    Reference UDPsocket_set_blocking(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<UDPSocket>();
            auto value = context["value"].to_data(context).get<bool>();

            boost::system::error_code ec;
            socket.non_blocking(!value, ec);

            if (!ec)
                return {};
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const UDPsocket_close_args = std::make_shared<Parser::Symbol>("socket");
    Reference UDPsocket_close(FunctionContext& context) {
        try {
            auto& socket = context["socket"].to_data(context).get<ObjectPtr>()->c_obj.get<UDPSocket>();

            boost::system::error_code ec;
            socket.close(ec);

            if (!ec)
                return {};
            else
                return Data(static_cast<OV_INT>(ec.value()));
        } catch (std::exception const&) {
            throw FunctionArgumentsError();
        }
    }


    auto const time_args = std::make_shared<Parser::Tuple>();
    Reference time(FunctionContext& /*context*/) {
        return Data(static_cast<OV_INT>(std::time(nullptr)));
    }

    auto const clock_system_args = std::make_shared<Parser::Tuple>();
    Reference clock_system(FunctionContext& /*context*/) {
        std::chrono::duration<double> d = std::chrono::system_clock::now().time_since_epoch();
        return Data(static_cast<OV_FLOAT>(d.count()));
    }

    auto const clock_steady_args = std::make_shared<Parser::Tuple>();
    Reference clock_steady(FunctionContext& /*context*/) {
        std::chrono::duration<double> d = std::chrono::steady_clock::now().time_since_epoch();
        return Data(static_cast<OV_FLOAT>(d.count()));
    }


#if defined(__cpp_lib_jthread)
    using Thread = std::jthread;
#else
    struct Thread : public std::thread {
        using std::thread::thread;
        using std::thread::operator=;

        ~Thread() {
            join();
        }
    };
#endif

    auto const thread_is_args = std::make_shared<Parser::Symbol>("thread");
    Reference thread_is(FunctionContext& context) {
        try {
            context["thread"].to_data(context).get<ObjectPtr>()->c_obj.get<Thread>();

            return Data(true);
        } catch (std::bad_any_cast const&) {
            return Data(false);
        } catch (Data::BadAccess const&) {
            return Data(false);
        }
    }

    auto const thread_create_args = std::make_shared<Parser::Symbol>("function");
    Reference thread_create(FunctionContext& context) {
        try {
            auto function = context["function"].to_data(context).get<ObjectPtr>();

            auto& global = context.get_global();
            auto caller = context.caller;

            auto obj = GC::new_object();
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

    auto const thread_join_args = std::make_shared<Parser::Symbol>("thread");
    Reference thread_join(FunctionContext& context) {
        try {
            auto& thread = context["thread"].to_data(context).get<ObjectPtr>()->c_obj.get<Thread>();
            thread.join();

            return {};
        } catch (std::bad_any_cast const&) {
            throw FunctionArgumentsError();
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const thread_detach_args = std::make_shared<Parser::Symbol>("thread");
    Reference thread_detach(FunctionContext& context) {
        try {
            auto& thread = context["thread"].to_data(context).get<ObjectPtr>()->c_obj.get<Thread>();
            thread.detach();

            return {};
        } catch (std::bad_any_cast const&) {
            throw FunctionArgumentsError();
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const thread_get_id_args = std::make_shared<Parser::Symbol>("thread");
    Reference thread_get_id(FunctionContext& context) {
        try {
            auto& thread = context["thread"].to_data(context).get<ObjectPtr>()->c_obj.get<Thread>();

            return Data(static_cast<OV_INT>(std::hash<std::thread::id>{}(thread.get_id())));
        } catch (std::bad_any_cast const&) {
            throw FunctionArgumentsError();
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const thread_current_id_args = std::make_shared<Parser::Tuple>();
    Reference thread_current_id(FunctionContext& /*unused*/) {
        return Data(static_cast<OV_INT>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    }

    auto const thread_sleep_args = std::make_shared<Parser::Symbol>("time");
    Reference thread_sleep(FunctionContext& context) {
        auto time = context["time"].to_data(context);

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

    auto const thread_hardware_concurrency_args = std::make_shared<Parser::Tuple>();
    Reference thread_hardware_concurrency(FunctionContext& /*unused*/) {
        return Data(static_cast<OV_INT>(std::thread::hardware_concurrency()));
    }


    auto const mutex_is_args = std::make_shared<Parser::Symbol>("mutex");
    Reference mutex_is(FunctionContext& context) {
        try {
            context["mutex"].to_data(context).get<ObjectPtr>()->c_obj.get<std::mutex>();

            return Data(true);
        } catch (std::bad_any_cast const&) {
            return Data(false);
        } catch (Data::BadAccess const&) {
            return Data(false);
        }
    }

    auto const mutex_create_args = std::make_shared<Parser::Tuple>();
    Reference mutex_create(FunctionContext&  /*context*/) {
        try {
            auto obj = GC::new_object();
            obj->c_obj.set(std::make_unique<std::mutex>());
            return Data(obj);
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const mutex_lock_args = std::make_shared<Parser::Symbol>("mutex");
    Reference mutex_lock(FunctionContext& context) {
        try {
            auto& mutex = context["mutex"].to_data(context).get<ObjectPtr>()->c_obj.get<std::mutex>();
            mutex.lock();
            return {};
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const mutex_try_lock_args = std::make_shared<Parser::Symbol>("mutex");
    Reference mutex_try_lock(FunctionContext& context) {
        try {
            auto& mutex = context["mutex"].to_data(context).get<ObjectPtr>()->c_obj.get<std::mutex>();
            return Data(mutex.try_lock());
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }

    auto const mutex_unlock_args = std::make_shared<Parser::Symbol>("mutex");
    Reference mutex_unlock(FunctionContext& context) {
        try {
            auto& mutex = context["mutex"].to_data(context).get<ObjectPtr>()->c_obj.get<std::mutex>();
            mutex.unlock();
            return {};
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        }
    }


    auto const GC_collect_args = std::make_shared<Parser::Tuple>();
    Reference GC_collect(FunctionContext& /*context*/) {
        GC::collect();
        return {};
    }


    void init(GlobalContext& context) {
        auto& s = context.get_global().system;
        s = GC::new_object();

        get_object(s.get_property("async"));

        add_function(s.get_property("stream_is"), stream_is_args, stream_is);
        add_function(s.get_property("stream_has"), stream_has_args, stream_has);
        add_function(s.get_property("istream_is"), istream_is_args, istream_is);
        add_function(s.get_property("stream_read"), stream_read1_args, stream_read1);
        add_function(s.get_property("stream_read"), stream_read2_args, stream_read2);
        add_function(s.get_property("stream_get_available"), stream_get_available_args, stream_get_available);
        add_function(s.get_property("stream_scan"), stream_scan_args, stream_scan);
        add_function(s.get_property("ostream_is"), ostream_is_args, ostream_is);
        add_function(s.get_property("stream_write"), stream_write1_args, stream_write1);
        add_function(s.get_property("stream_write"), stream_write2_args, stream_write2);
        add_function(s.get_property("stream_flush"), stream_flush_args, stream_flush);
        add_function(s.get_property("stream_print"), stream_print_args, stream_print);

        add_function(s.get_property("file_is"), file_is_args, file_is);
        add_function(s.get_property("file_open"), file_path_args, file_open);
        add_function(s.get_property("file_close"), file_close_args, file_close);
        add_function(s.get_property("file_get_current_directory"), std::make_shared<Parser::Tuple>(), file_get_current_directory);
        add_function(s.get_property("file_set_current_directory"), file_path_args, file_set_current_directory);
        add_function(s.get_property("file_exists"), file_path_args, file_exists);
        add_function(s.get_property("file_size"), file_path_args, file_size);
        add_function(s.get_property("file_is_empty"), file_path_args, file_is_empty);
        add_function(s.get_property("file_is_directory"), file_path_args, file_is_directory);
        add_function(s.get_property("file_create_directories"), file_path_args, file_create_directories);
        add_function(s.get_property("file_copy"), file_copy_args, file_copy);
        add_function(s.get_property("file_rename"), file_copy_args, file_rename);
        add_function(s.get_property("file_delete"), file_path_args, file_delete);
        add_function(s.get_property("file_children"), file_path_args, file_children);
        add_function(s.get_property("file_concatenate"), file_copy_args, file_concatenate);
        add_function(s.get_property("file_parent"), file_path_args, file_parent);
        add_function(s.get_property("file_absolute"), file_path_args, file_absolute);
        add_function(s.get_property("file_root"), file_path_args, file_root);
        add_function(s.get_property("file_filename"), file_path_args, file_filename);
        add_function(s.get_property("file_extension"), file_path_args, file_extension);
        add_function(s.get_property("file_filename_without_extension"), file_path_args, file_filename_without_extension);

        add_function(s.get_property("TCPsocket_is"), TCPsocket_is_args, TCPsocket_is);
        add_function(s.get_property("TCPsocket_connect"), TCPsocket_connect_args, TCPsocket_connect);
        add_function(s.get_property("TCPsocket_receive"), TCPsocket_receive_args, TCPsocket_receive);
        add_function(s.get_property("TCPsocket_send"), TCPsocket_send_args, TCPsocket_send);
        add_function(s.get_property("TCPsocket_get_blocking"), TCPsocket_get_blocking_args, TCPsocket_get_blocking);
        add_function(s.get_property("TCPsocket_set_blocking"), TCPsocket_set_blocking_args, TCPsocket_set_blocking);
        add_function(s.get_property("TCPsocket_close"), TCPsocket_close_args, TCPsocket_close);

        add_function(s.get_property("TCPacceptor_is"), TCPacceptor_is_args, TCPacceptor_is);
        add_function(s.get_property("TCPacceptor_bind"), TCPacceptor_bind_args, TCPacceptor_bind);
        add_function(s.get_property("TCPacceptor_accept"), TCPacceptor_accept_args, TCPacceptor_accept);
        add_function(s.get_property("TCPacceptor_get_blocking"), TCPacceptor_get_blocking_args, TCPacceptor_get_blocking);
        add_function(s.get_property("TCPacceptor_set_blocking"), TCPacceptor_set_blocking_args, TCPacceptor_set_blocking);
        add_function(s.get_property("TCPacceptor_close"), TCPacceptor_close_args, TCPacceptor_close);

        add_function(s.get_property("UDPsocket_is"), UDPsocket_is_args, UDPsocket_is);
        add_function(s.get_property("UDPsocket_bind"), UDPsocket_bind_args, UDPsocket_bind);
        add_function(s.get_property("UDPsocket_open"), UDPsocket_open_args, UDPsocket_open);
        add_function(s.get_property("UDPsocket_receive_from"), UDPsocket_receive_from_args, UDPsocket_receive_from);
        add_function(s.get_property("UDPsocket_send_to"), UDPsocket_send_to_args, UDPsocket_send_to);
        add_function(s.get_property("UDPsocket_get_blocking"), UDPsocket_get_blocking_args, UDPsocket_get_blocking);
        add_function(s.get_property("UDPsocket_set_blocking"), UDPsocket_set_blocking_args, UDPsocket_set_blocking);
        add_function(s.get_property("UDPsocket_close"), UDPsocket_close_args, UDPsocket_close);

        add_function(s.get_property("time"), time_args, time);
        add_function(s.get_property("clock_system"), clock_system_args, clock_system);
        add_function(s.get_property("clock_steady"), clock_steady_args, clock_steady);

        add_function(s.get_property("thread_is"), thread_is_args, thread_is);
        add_function(s.get_property("thread_create"), thread_create_args, thread_create);
        add_function(s.get_property("thread_join"), thread_join_args, thread_join);
        add_function(s.get_property("thread_detach"), thread_detach_args, thread_detach);
        add_function(s.get_property("thread_get_id"), thread_get_id_args, thread_get_id);
        add_function(s.get_property("thread_current_id"), thread_current_id_args, thread_current_id);
        add_function(s.get_property("thread_sleep"), thread_sleep_args, thread_sleep);
        add_function(s.get_property("thread_hardware_concurrency"), thread_hardware_concurrency_args, thread_hardware_concurrency);

        add_function(s.get_property("mutex_is"), mutex_is_args, mutex_is);
        add_function(s.get_property("mutex_create"), mutex_create_args, mutex_create);
        add_function(s.get_property("mutex_lock"), mutex_lock_args, mutex_lock);
        add_function(s.get_property("mutex_try_lock"), mutex_try_lock_args, mutex_try_lock);
        add_function(s.get_property("mutex_unlock"), mutex_unlock_args, mutex_unlock);

        add_function(s.get_property("GC_collect"), GC_collect_args, GC_collect);

        get_object(s.get_property("in"))->c_obj.set(std::reference_wrapper<std::ios>(std::cin));
        get_object(s.get_property("out"))->c_obj.set(std::reference_wrapper<std::ios>(std::cout));
        get_object(s.get_property("err"))->c_obj.set(std::reference_wrapper<std::ios>(std::cerr));
    }

}
