#include "Config_Server.hpp"
#include <Mlib/Json/Json_Object_File.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Remote/Config_Server/IHttp_Response_Generator.hpp>
#include <Mlib/Remote/Config_Server/Request_Overrides.hpp>
#include <Mlib/Remote/Datagram_Nodes/Datagram_Node_Factory.hpp>
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <Mlib/Remote/Sockets/Websocket.hpp>
#include <Mlib/Strings/Base64.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
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
    Utf8Path static_dir,
    std::vector<std::shared_ptr<IHttpResponseGenerator>> response_generators,
    std::shared_ptr<IHttpResponseGenerator> error_generator)
    : static_dir_{std::move(static_dir)}
    , response_generators_{std::move(response_generators)}
    , error_generator_{std::move(error_generator)}
    , reload_required_{false}
    , http_thread_{[this, remote_socket](){
        auto const address = asio::ip::make_address(remote_socket.hostname);
        asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {address, remote_socket.port}};
        linfo() << "Configuration server started at http://" << remote_socket;
        acceptor.native_non_blocking(false);
        try {
            while (!unhandled_exceptions_occured() &&
                   !http_thread_.get_stop_token().stop_requested())
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
        ioc.run();
    }}
{
    cert_hash_ = []() -> std::string {
        auto filename = try_getenv("CERT_HASH_FILENAME");
        if (filename.has_value()) {
            linfo() << "Reading CERT_HASH_FILENAME";
            JsonObjectFile obj;
            obj.load_from_file(*filename);
            auto s = obj.at<std::string>("hash");
            auto v = decode_base64(s);
            if (v.size() != 32) {
                throw std::runtime_error("Cert hash does not have 32 bytes");
            }
            linfo() << "CERT_HASH_FILENAME has been read";
            return s;
        }
        linfo() << "CERT_HASH_FILENAME not set";
        return "";
    }();
}

ConfigServer::~ConfigServer() {
    http_thread_.get_stop_token().request_stop();
}

void ConfigServer::handle_session(tcp::socket socket) {
    auto shared_socket = std::make_shared<tcp::socket>(std::move(socket));
    auto ws_stream = std::make_shared<std::optional<websocket::stream<tcp::socket>>>();

    DestructionGuard guard([shared_socket, ws_stream]() {
        boost::beast::error_code ec;
        
        // Path A: A WebSocket upgrade attempt was made, and it failed
        if (*ws_stream && (*ws_stream)->next_layer().is_open()) {
            (*ws_stream)->next_layer().shutdown(tcp::socket::shutdown_both, ec);
            (*ws_stream)->next_layer().close(ec);
        } 
        // Path B: Normal HTTP mode
        else if (shared_socket && shared_socket->is_open()) {
            shared_socket->shutdown(tcp::socket::shutdown_both, ec);
            shared_socket->close(ec);
        }
    });

    beast::flat_buffer buffer;
    
    for(;;) {
        http::request<http::string_body> req;
        boost::beast::error_code ec;
        
        http::read(*shared_socket, buffer, req, ec);
        
        if (ec == boost::beast::http::error::end_of_stream || ec) {
            if (ec && ec != boost::beast::http::error::end_of_stream) {
                lerr() << "Read error: " << ec.message();
            }
            break; 
        }

        if (websocket::is_upgrade(req)) {
            linfo() << "Handle request to upgrade to websockets";
            
            ws_stream->emplace(std::move(*shared_socket));

            beast::error_code ws_ec;
            (*ws_stream)->accept(req, ws_ec);
            
            if (ws_ec.failed()) {
                lerr() << "Websocket accept failed: " << ws_ec.message();
                return;
            }

            std::scoped_lock lock{receive_mutex_};
            
            websocket::stream<tcp::socket> final_ws = std::move(**ws_stream);
            ws_stream->reset(); // Deactivate WebSocket path for the guard

            auto& node = websocket_nodes_.emplace_back(DatagramNodeFactory::create_websocket(std::move(final_ws)));
            node->start_receive_thread(500); 
            return; 
        } else {
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

            auto send_message = [&shared_socket](boost::beast::http::message_generator msg){
                beast::error_code write_ec;
                beast::write(*shared_socket, std::move(msg), write_ec);
                if (write_ec.failed()) {
                    lerr() << "Failure during send: " << write_ec.message();
                }
            };

            bool keep_alive = req.keep_alive();
            bool message_sent = false;
            http::status status = http::status::not_found;
            
            auto url_res = boost::urls::parse_origin_form(req.target());
            if (url_res.has_error()) {
                lerr() << "URL parsing failed: " << url_res.error().message();
                status = http::status::bad_request; 
            } else {
                RequestOverrides reo{headers, req, url_res->path(), concatenate(static_dir_, url_res->path()), std::nullopt};
                for (const auto& g : response_generators_) {
                    auto response = g->reply(reo);
                    if (auto* message = std::get_if<http::message_generator>(&response)) {
                        send_message(std::move(*message));
                        message_sent = true;
                        break; 
                    }
                    if (auto* s = std::get_if<http::status>(&response)) {
                        status = *s;
                        break;
                    }
                }
            }

            if (!message_sent) {
                RequestOverrides reo{headers, req, "server/error.html", concatenate(static_dir_, "server", "error.html"), status};
                auto response = error_generator_->reply(reo);
                if (auto* message = std::get_if<http::message_generator>(&response)) {
                    send_message(std::move(*message));
                } else {
                    lerr() << "Close after failed error handling";
                    return;
                }
            }

            if (!keep_alive) {
                break; 
            }
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

std::shared_ptr<ISendSocket> ConfigServer::try_receive(std::ostream& ostr) {
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
        auto socket = node->object()->try_receive(ostr);
        websocket_nodes_in_progress_->erase(node);
        if (socket != nullptr) {
            return socket;
        }
    }
    websocket_nodes_in_progress_.reset();
    return nullptr;
}
