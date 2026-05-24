#include "Http_Session.hpp"
#include <Mlib/Remote/Config_Server/IRequest_Handler.hpp>
#include <Mlib/Remote/Sockets/Websocket.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

using namespace Mlib;

HttpSession::HttpSession(tcp::socket socket, IRequestHandler& handler)
    : socket_(std::move(socket))
    , keep_alive_{true}
    , handler_(handler)
{}

void HttpSession::run() {
    do_read();
}

void HttpSession::do_read() {
    auto self = shared_from_this();
    
    http::async_read(socket_, buffer_, req_, [self](beast::error_code ec, std::size_t) {
        if (ec) {
            return; 
        }

        if (beast::websocket::is_upgrade(self->req_)) {
            self->handler_.handle_websocket_upgrade(std::move(self->req_), std::move(self->socket_));
            return;
        }

        http::message_generator response = self->handler_.handle_request(std::move(self->req_));
        self->keep_alive_ = response.keep_alive();

        self->do_write(std::move(response));
    });
}

void HttpSession::do_write(http::message_generator msg) {
    auto self = shared_from_this();
    beast::async_write(socket_, std::move(msg), [self](beast::error_code ec, std::size_t) {
        if (ec) {
            return; 
        }

        // ERST WENN DIE ANTWORT C_OK BEIM CLIENT IST:
        // Prüfen wir, ob die Verbindung offen bleiben soll, und lesen den NÄCHSTEN Request.
        if (self->keep_alive_) {
            self->do_read();
        }
    });
}
