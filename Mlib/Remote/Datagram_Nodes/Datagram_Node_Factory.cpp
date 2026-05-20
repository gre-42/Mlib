#include "Datagram_Node_Factory.hpp"
#include <Mlib/Remote/Datagram_Nodes/Threaded_Datagram_Node.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <Mlib/Remote/Sockets/Websocket_Socket.hpp>
#ifdef __EMSCRIPTEN__
#include <Mlib/Remote/Datagram_Nodes/Web_Transport_Datagram_Node.hpp>
#else
#include <Mlib/Remote/Sockets/Asio.hpp>
#include <Mlib/Remote/Sockets/Udp_Socket.hpp>

using boost::asio::ip::address;
using boost::asio::ip::udp;
#endif

using namespace Mlib;

#ifdef __EMSCRIPTEN__
std::shared_ptr<IDatagramNode> DatagramNodeFactory::create_web_transport(
    const RemoteSocket& socket,
    std::vector<std::byte> cert_hash)
{
    return WebTransportDatagramNode::create(socket, std::move(cert_hash));
}
#else
std::shared_ptr<IDatagramNode> DatagramNodeFactory::create_udp(
    boost::asio::io_context& io_context,
    const RemoteSocket& socket)
{
    auto endpoint = udp::endpoint{ address{boost::asio::ip::make_address_v4(socket.hostname)}, socket.port };
    auto boost_socket = std::make_shared<udp::socket>(io_context);
    auto mlib_socket = std::make_shared<UdpSocket>(udp::v4(), std::move(boost_socket), endpoint);
    mlib_socket->open();
    return std::make_shared<ThreadedDatagramNode>(mlib_socket);
}
#endif

std::shared_ptr<IDatagramNode> DatagramNodeFactory::create_websocket(
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket)
{
    auto mlib_socket = std::make_shared<WebsocketSocket>(std::move(socket));
    return std::make_shared<ThreadedDatagramNode>(mlib_socket);
}
