#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <queue>

namespace Mlib {

class IRequestHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
private:
    boost::asio::ip::tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    bool keep_alive_ = true;
    IRequestHandler& handler_;

public:
    HttpSession(boost::asio::ip::tcp::socket socket, IRequestHandler& handler);
    void run();

private:
    void do_read();
    void do_write(boost::beast::http::message_generator msg);
};

}
