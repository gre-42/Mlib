#pragma once
#include <Mlib/Remote/Sockets/Asio.hpp>
#include <Mlib/Remote/Sockets/IDatagram_Socket.hpp>
#include <Mlib/Remote/Sockets/Websocket.hpp>

namespace Mlib {

class WebsocketSocket: public IDatagramSocket {
public:
    explicit WebsocketSocket(
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket);
    virtual void open() override;
    virtual void bind() override;
    virtual void shutdown(std::error_code& ec) override;
    virtual void close() override;
    virtual size_t receive(
        std::vector<std::byte>& receive_buffer,
        std::shared_ptr<IDatagramSocket>& reply_socket,
        std::error_code& ec) override;
    virtual size_t send(
        const std::vector<std::byte>& data,
        std::error_code& ec) override;
private:
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket_;
};

}
