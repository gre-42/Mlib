#include "Config_Server.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/url/parse_path.hpp>
#include <concepts>
#include <map>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

using namespace Mlib;

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

template <typename T>
concept UrlSegment = std::convertible_to<T, boost::core::string_view>;

template <UrlSegment... Args>
static Utf8Path concatenate(Utf8Path root, Args&&... targets) {
    auto process_one = [&](boost::core::string_view target) {
        for (auto const& seg : boost::urls::parse_path(target).value()) {
            if ((seg == ".") || (seg == "..")) {
                throw std::runtime_error("Detected illegal character in URL");
            }
            root /= std::string(seg);
        }
    };

    (process_one(boost::core::string_view(std::forward<Args>(targets))), ...);
    return root;
}

bool wait_for_connection(
    boost::asio::io_context& io_context,
    tcp::acceptor& acceptor,
    std::chrono::milliseconds timeout)
{
    acceptor.non_blocking(true);
    
    bool ready = false;
    bool expired = false;
    std::error_code select_ec = std::make_error_code(std::errc::operation_in_progress);

    // Start asynchronous wait on the socket
    acceptor.async_wait(tcp::acceptor::wait_read, [&](const std::error_code& ec) {
        if (!expired) {
            ready = true;
            select_ec = ec;
        }
    });

    // Start a timer for the timeout duration
    boost::asio::steady_timer timer(io_context, timeout);
    timer.async_wait([&](const std::error_code& ec) {
        if (!ec && !ready) {
            expired = true;
            acceptor.cancel(); // Cancel the pending socket wait
        }
    });

    // Block the thread until one of the operations completes
    io_context.restart();
    io_context.run_for(timeout + std::chrono::milliseconds(10)); 

    // Evaluate outcomes
    if (expired || (select_ec == std::errc::interrupted)) {
        return false;
    }
    if (select_ec) {
        throw std::system_error(select_ec, "Wait failed in configuration server");
    }

    return true; // Connection is ready to accept
}

ConfigServer::ConfigServer(
    const RemoteSocket& remote_socket,
    Utf8Path static_dir)
    : static_dir_{std::move(static_dir)}
    , reload_required_{false}
    , thread_{[this, remote_socket](){
        auto const address = asio::ip::make_address(remote_socket.hostname);
        asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {address, remote_socket.port}};
        linfo() << "Configuration server started at http://" << remote_socket;
        acceptor.native_non_blocking(false);
        try {
            while (!unhandled_exceptions_occured() &&
                   !thread_.get_stop_token().stop_requested())
            {
                if (!wait_for_connection(ioc, acceptor, std::chrono::milliseconds{500})) {
                    continue;
                }
                tcp::socket socket{ioc};
                boost::system::error_code ec;
                acceptor.accept(socket, ec);
                if (ec) {
                    throw boost::system::system_error(ec); 
                }
                handle_session(std::move(socket));
            }
        } catch (const std::exception& e) {
            lerr() << "Unhandled exception in configuration server: " << e.what();
            add_unhandled_exception(std::current_exception());
        } catch (...) {
            lerr() << "Unknown unhandled exception in configuration server";
            add_unhandled_exception(std::current_exception());
        }
        linfo() << "Exit configuration server";
        notify_reload_required();
    }}
{}

ConfigServer::~ConfigServer() {
    thread_.get_stop_token().request_stop();
}

void ConfigServer::handle_session(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;

        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        if (websocket::is_upgrade(req)) {
            websocket::stream<tcp::socket> ws{std::move(socket)};

            ws.accept(req);

            // Handle WebSocket messages (simple echo)
            while (true) {
                beast::flat_buffer ws_buffer;
                ws.read(ws_buffer);
                ws.text(ws.got_text());
                ws.write(ws_buffer.data());
            }
        } else {
            // Handle as standard HTTP
            auto send_file = [&req, &socket](
                const Utf8Path& path,
                const std::string& content_type,
                http::status status,
                const std::vector<std::pair<http::field, std::string>>* headers = nullptr)
            {
                linfo() << "Send file \"" << path.string() << '"';
                http::file_body::value_type body;
                beast::error_code ec;
                body.open(path.c_str(), beast::file_mode::scan, ec);
                if (ec.failed()) {
                    lerr() << "Could not open file \"" << path.string() << '"';
                    return false;
                }
                if (body.size() == 0) {
                    lerr() << "File is empty: \"" << path.string() << '"';
                    return false;
                }
                http::response<http::file_body> res{
                    std::piecewise_construct,
                    std::make_tuple(std::move(body)),
                    std::make_tuple(status, req.version())
                };
                res.set(http::field::content_type, content_type);
                if (headers != nullptr) {
                    for (const auto& h : *headers) {
                        res.set(h.first, h.second);
                    }
                }
                res.content_length(body.size());
                res.prepare_payload();
                linfo() << "Write file body: \"" << path.string() << '"';
                http::write(socket, res);
                linfo() << "File body written: \"" << path.string() << '"';
                return true;
            };
            auto send_error_html = [&](http::status status){
                auto e = concatenate(static_dir_, "server", "error.html");
                if (!send_file(e, "text/html", status)) {
                    lerr() << "Close the socket after failed error handling";
                    boost::beast::error_code ec;
                    // Tell the OS to stop sending/receiving and drop buffered data
                    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                    socket.close(ec);
                }
            };
            static const std::map<std::string, std::string> redirects = {
                {"/", "server/index.html"}
            };
            auto redirect = redirects.find(req.target());
            auto target = (redirect != redirects.end())
                ? boost::beast::string_view{redirect->second}
                : req.target();
            auto p = concatenate(static_dir_, target);
            auto extension = p.extension();
            static const std::map<std::string, std::string> mime_types = {
                {".html", "text/html"},
                {".js", "text/javascript"},
                {".data", "application/octet-stream"},
                {".wasm", "application/wasm"},
            };
            auto cross_origin = std::vector<std::pair<http::field, std::string>>{
                {http::field::cross_origin_opener_policy, "same-origin"},
                {http::field::cross_origin_embedder_policy, "require-corp"}
            };
            static const std::map<std::string, std::vector<std::pair<http::field, std::string>>> headers = {
                {".html", cross_origin},
                {".js", cross_origin},
                {".wasm", cross_origin},
                {".data", cross_origin},
            };
            auto mime_type = mime_types.find(extension);
            if (mime_type != mime_types.end()) {
                auto header_it = headers.find(extension);
                const auto* header = (header_it != headers.end())
                    ? &header_it->second
                    : nullptr;
                if (!send_file(p, mime_type->second, http::status::ok, header)) {
                    send_error_html(http::status::not_found);
                }
            } else {
                send_error_html(http::status::not_implemented);
            }
        }
    } catch (beast::system_error const& se) {
        if (se.code() != websocket::error::closed) {
            lwarn() << se.code().message();
        }
    }
}

bool ConfigServer::application_should_exit() const {
    return false;
}

void ConfigServer::notify_reload_required() {
    reload_required_ = true;
    cv_.notify_all();
}

void ConfigServer::wait_until_reload_required() const {
    std::unique_lock lock{mutex_};
    cv_.wait(lock, [this](){ return reload_required_; });
}
