#pragma once
#include <Mlib/Memory/Dangling_List.hpp>
#include <Mlib/Remote/Config_Server/IRequest_Handler.hpp>
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace Mlib {

struct RemoteSocket;
class ISendSocket;
class IDatagramNode;
class IHttpResponseGenerator;

class ConfigServer: public IReceiveSocket, public IRequestHandler {
public:
    ConfigServer(
        const RemoteSocket& remote_socket,
        Utf8Path static_dir,
        std::vector<std::shared_ptr<IHttpResponseGenerator>> response_generators,
        std::shared_ptr<IHttpResponseGenerator> error_generator);
    ~ConfigServer();
    bool application_should_exit() const;
    // IReceiveSocket
    virtual std::shared_ptr<ISendSocket> try_receive(
        std::ostream& ostr,
        NetworkTransmissionStatus& transmission_status) override;
    // IRequestHandler
    virtual boost::beast::http::message_generator handle_request(
        boost::beast::http::request<boost::beast::http::string_body> req) override;
    virtual void handle_websocket_upgrade(
        boost::beast::http::request<boost::beast::http::string_body> req,
        boost::asio::ip::tcp::socket socket) override;
private:
    void handle_session(boost::asio::ip::tcp::socket socket);
    Utf8Path static_dir_;
    std::vector<std::shared_ptr<IHttpResponseGenerator>> response_generators_;
    std::shared_ptr<IHttpResponseGenerator> error_generator_;
    std::string cert_hash_;
    mutable std::mutex mutex_;
    mutable FastMutex receive_mutex_;
    std::list<std::shared_ptr<IDatagramNode>> websocket_nodes_;
    std::optional<DanglingList<IDatagramNode>> websocket_nodes_in_progress_;
    boost::asio::io_context ioc_;
    JThread http_thread_;
};

}
