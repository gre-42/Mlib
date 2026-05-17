#pragma once
#include <Mlib/Remote/Sockets/Websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#ifndef __EMSCRIPTEN__
#include <Mlib/Remote/Sockets/Asio.hpp>
#endif

namespace Mlib {

struct RemoteSocket;
class IDatagramNode;

namespace DatagramNodeFactory {

#ifdef __EMSCRIPTEN__
 std::shared_ptr<IDatagramNode> create_web_transport(
    const RemoteSocket& socket);
#else
std::shared_ptr<IDatagramNode> create_udp(
    boost::asio::io_context& io_context,
    const RemoteSocket& socket);
#endif
std::shared_ptr<IDatagramNode> create_websocket(
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket);

}

}
