#pragma once
#include <Mlib/Remote/Sockets/Asio.hpp>
#include <Mlib/Remote/Sockets/IDatagram_Socket.hpp>

namespace Mlib {

class UdpSocket: public IDatagramSocket {
public:
    UdpSocket(
        boost::asio::ip::udp protocol,
        std::shared_ptr<boost::asio::ip::udp::socket> socket,
        boost::asio::ip::udp::endpoint endpoint);
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
    boost::asio::ip::udp protocol_;
    std::shared_ptr<boost::asio::ip::udp::socket> socket_;
    boost::asio::ip::udp::endpoint endpoint_;
};

}
