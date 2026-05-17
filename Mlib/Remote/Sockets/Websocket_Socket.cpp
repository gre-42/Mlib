#include "Websocket_Socket.hpp"

using namespace Mlib;

WebsocketSocket::WebsocketSocket(
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket)
    : socket_{ std::move(socket) }
{}

void WebsocketSocket::open() {
    throw std::runtime_error("Websocket cannot be opened");
}

void WebsocketSocket::bind() {
    throw std::runtime_error("Websocket cannot be bound");
}

void WebsocketSocket::shutdown(std::error_code& ec) {
    throw std::runtime_error("Websocket cannot be shutdown");
}

void WebsocketSocket::close() {
    throw std::runtime_error("Websocket cannot be closed");
}

size_t WebsocketSocket::receive(
    std::vector<std::byte>& receive_buffer,
    std::shared_ptr<IDatagramSocket>& reply_socket,
    std::error_code& ec)
{
    boost::beast::flat_buffer ws_buffer;
    boost::system::error_code boost_ec;
    auto len = socket_.read(ws_buffer, boost_ec);
    ec = boost_ec;
    if (ec) {
        return 0;
    }
    const auto& data = ws_buffer.data();
    if (len > receive_buffer.capacity()) {
        throw std::runtime_error("Websocket datagram too large");
    }
    std::copy(
        (const std::byte*)data.data(),
        (const std::byte*)data.data() + len,
        receive_buffer.data());
    return len;
}

size_t WebsocketSocket::send(
    const std::vector<std::byte>& data,
    std::error_code& ec)
{
    boost::system::error_code boost_ec;
    auto res = socket_.write(boost::asio::buffer(data), boost_ec);
    ec = boost_ec;
    return res;
}
