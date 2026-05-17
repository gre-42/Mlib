#include "Udp_Socket.hpp"

using namespace Mlib;

UdpSocket::UdpSocket(
    boost::asio::ip::udp protocol,
    std::shared_ptr<boost::asio::ip::udp::socket> socket,
    boost::asio::ip::udp::endpoint endpoint)
    : protocol_{ protocol }
    , socket_{ std::move(socket) }
    , endpoint_{ std::move(endpoint) }
{}

void UdpSocket::open() {
    socket_->open(protocol_);
}

void UdpSocket::bind() {
    socket_->bind(endpoint_);
}

void UdpSocket::shutdown(std::error_code& ec) {
    boost::system::error_code boost_ec;
    socket_->shutdown(boost::asio::socket_base::shutdown_both, boost_ec);
    ec = boost_ec;
}

void UdpSocket::close() {
    socket_->close();
}

size_t UdpSocket::receive(
    std::vector<std::byte>& receive_buffer,
    std::shared_ptr<IDatagramSocket>& reply_socket,
    std::error_code& ec)
{
    boost::asio::ip::udp::endpoint endpoint2;
    boost::system::error_code boost_ec;
    auto len = socket_->receive_from(
        boost::asio::buffer(receive_buffer),
        endpoint2,
        0,
        boost_ec);
    ec = boost_ec;
    if (ec) {
        return 0;
    }
    reply_socket = std::make_shared<UdpSocket>(protocol_, socket_, endpoint2);
    return len;
}

size_t UdpSocket::send(
    const std::vector<std::byte>& data,
    std::error_code& ec)
{
    boost::system::error_code boost_ec;
    auto res = socket_->send_to(boost::asio::buffer(data), endpoint_, 0, boost_ec);
    ec = boost_ec;
    return res;
}
