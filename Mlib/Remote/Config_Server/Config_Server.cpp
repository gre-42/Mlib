#include "Config_Server.hpp"
#include <Mlib/Json/Json_Object_File.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Remote/Config_Server/Http_Session.hpp>
#include <Mlib/Remote/Config_Server/IHttp_Response_Generator.hpp>
#include <Mlib/Remote/Config_Server/Request_Overrides.hpp>
#include <Mlib/Remote/Datagram_Nodes/Datagram_Node_Factory.hpp>
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Remote/Network_Transmission_Status.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <Mlib/Remote/Sockets/Websocket.hpp>
#include <Mlib/Strings/Base64.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Safe_Promise.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/parse_path.hpp>
#include <boost/url/url_view.hpp>

using namespace Mlib;

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

template <typename T>
concept UrlSegment = std::convertible_to<T, boost::urls::url_view>;

template <UrlSegment... Args>
static Utf8Path concatenate(Utf8Path root, Args&&... targets) {
    auto process_one = [&](boost::core::string_view target) {
        auto parsed = boost::urls::parse_path(target);
        if (parsed.has_error()) {
            throw std::system_error(parsed.error(), (std::stringstream() << "Could not parse \"" << target << '"').str());
        }
        for (auto const& seg : *parsed) {
            if ((seg == ".") || (seg == "..")) {
                throw std::runtime_error("Detected illegal character in URL");
            }
            root /= Utf8Path{seg};
        }
    };

    (process_one(boost::core::string_view(std::forward<Args>(targets))), ...);
    return root;
}

ConfigServer::ConfigServer(
    const RemoteSocket& remote_socket,
    Utf8Path static_dir,
    std::vector<std::shared_ptr<IHttpResponseGenerator>> response_generators,
    std::shared_ptr<IHttpResponseGenerator> error_generator)
    : static_dir_{std::move(static_dir)}
    , response_generators_{std::move(response_generators)}
    , error_generator_{std::move(error_generator)}
    , ioc_{1}
    , http_thread_{[this, remote_socket](){
        auto const address = asio::ip::make_address(remote_socket.hostname);
        
        auto acceptor = std::make_shared<tcp::acceptor>(ioc_, tcp::endpoint{address, remote_socket.port});
        
        linfo() << "Configuration server started at http://" << remote_socket;
        
        // Definition of the asynchronous accept loop (without use of 'ioc_')
        std::function<void()> do_accept = [this, acceptor, &do_accept]() {
            if (http_thread_.get_stop_token().stop_requested() || unhandled_exceptions_occured()) {
                acceptor->close(); 
                return;
            }

            acceptor->async_accept([this, acceptor, &do_accept](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    handle_session(std::move(socket));
                } else if (ec != asio::error::operation_aborted) {
                    lerr() << "Accept error: " << ec.message();
                    add_unhandled_exception(std::make_exception_ptr(boost::system::system_error(ec)));
                    acceptor->close();
                    return;
                }

                // Initiates the next asynchronous accept process
                do_accept();
            });
        };

        try {
            // Initiates the chain
            do_accept();

            // The event loop blocks the thread and processes in the background
            ioc_.run();

        } catch (const std::exception& e) {
            lerr() << "Unhandled exception in configuration server: " << e.what();
            add_unhandled_exception(std::current_exception());
        } catch (...) {
            lerr() << "Unknown unhandled exception in configuration server";
            add_unhandled_exception(std::current_exception());
        }
        linfo() << "Exit configuration server";
    }}
{}

ConfigServer::~ConfigServer() {
    http_thread_.get_stop_token().request_stop();
    ioc_.stop();
}

boost::beast::http::message_generator ConfigServer::handle_request(
    boost::beast::http::request<boost::beast::http::string_body> req)
{
    #ifdef __EMSCRIPTEN__
    static const auto headers = std::make_shared<std::vector<std::pair<std::string, std::string>>>(
        std::vector<std::pair<std::string, std::string>>{
            {"Cross-Origin-Opener-Policy", "same-origin"},
            {"Cross-Origin-Embedder-Policy", "require-corp"}
        });
    #else
    static const auto headers = std::make_shared<std::vector<std::pair<http::field, std::string>>>(
        std::vector<std::pair<http::field, std::string>>{
            {http::field::cross_origin_opener_policy, "same-origin"},
            {http::field::cross_origin_embedder_policy, "require-corp"}
        });
    #endif

    http::status status = http::status::not_found;
    
    auto url_res = boost::urls::parse_origin_form(req.target());
    if (url_res.has_error()) {
        lerr() << "URL parsing failed: " << url_res.error().message();
        status = http::status::bad_request; 
    } else {
        auto url_path = url_res->path();
        RequestOverrides reo{headers, req, url_path, concatenate(static_dir_, url_path), std::nullopt};
        
        for (const auto& g : response_generators_) {
            auto response = g->reply(reo);
            
            if (auto* message = std::get_if<http::message_generator>(&response)) {
                return std::move(*message); 
            }
            
            if (auto* s = std::get_if<http::status>(&response)) {
                status = *s;
                break;
            }
        }
    }

    RequestOverrides reo{headers, req, "server/error.html", concatenate(static_dir_, "server", "error.html"), status};
    auto response = error_generator_->reply(reo);
    
    if (auto* message = std::get_if<http::message_generator>(&response)) {
        return std::move(*message);
    } 

    lerr() << "Close after failed error handling";
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Internal Server Error";
    res.prepare_payload();
    return res;
}

void ConfigServer::handle_websocket_upgrade(
    boost::beast::http::request<boost::beast::http::string_body> req,
    boost::asio::ip::tcp::socket socket)
{
    linfo() << "Handle request to upgrade to websockets";

    auto ws_stream = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));

    ws_stream->async_accept(std::move(req), 
        [this, ws_stream](beast::error_code ws_ec) {
            if (ws_ec.failed()) {
                lerr() << "Websocket async accept failed: " << ws_ec.message();
                
                // Explicitly close in case of an error
                beast::error_code ec;
                ws_stream->next_layer().shutdown(tcp::socket::shutdown_both, ec);
                ws_stream->next_layer().close(ec);
                return;
            }

            linfo() << "Websocket async accept successful";

            // Success: Lock and add to nodes
            std::scoped_lock lock{receive_mutex_};
            
            auto& node = websocket_nodes_.emplace_back(
                DatagramNodeFactory::create_websocket(std::move(*ws_stream))
            );
            
            node->start_receive_thread(500);
        });
}

void ConfigServer::handle_session(tcp::socket socket) {
    // This is non-blocking, so ioc.run() can process other sockets.
    std::make_shared<HttpSession>(std::move(socket), *this)->run();
}

bool ConfigServer::application_should_exit() const {
    return false;
}

std::shared_ptr<ISendSocket> ConfigServer::try_receive(
    std::ostream& ostr,
    NetworkTransmissionStatus& transmission_status)
{
    std::scoped_lock lock{receive_mutex_};
    if (!websocket_nodes_in_progress_.has_value()) {
        websocket_nodes_in_progress_.emplace();
        for (auto& n : websocket_nodes_) {
            websocket_nodes_in_progress_->emplace_back(
                DanglingBaseClassRef<IDatagramNode>{*n, CURRENT_SOURCE_LOCATION},
                CURRENT_SOURCE_LOCATION);
        }
    }
    while (!websocket_nodes_in_progress_->empty()) {
        auto node = websocket_nodes_in_progress_->begin();
        auto socket = node->object()->try_receive(ostr, transmission_status);
        websocket_nodes_in_progress_->erase(node);
        if (socket != nullptr) {
            return socket;
        }
    }
    websocket_nodes_in_progress_.reset();
    transmission_status = NetworkTransmissionStatus::SUCCESS;
    return nullptr;
}
