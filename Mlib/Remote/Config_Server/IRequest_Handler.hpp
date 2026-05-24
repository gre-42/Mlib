#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body.hpp>
#include <memory>

namespace Mlib {

class IRequestHandler {
public:
    virtual ~IRequestHandler() = default;

    virtual boost::beast::http::message_generator handle_request(
        boost::beast::http::request<boost::beast::http::string_body> req) = 0;

    virtual void handle_websocket_upgrade(
        boost::beast::http::request<boost::beast::http::string_body> req,
        boost::asio::ip::tcp::socket socket) = 0;
};

}
