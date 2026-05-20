#pragma once
#include <Mlib/Memory/Dangling_List.hpp>
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>

namespace Mlib {

class ISendSocket;
struct RemoteSocket;
class IDatagramNode;

class ConfigServer: public IReceiveSocket {
public:
    explicit ConfigServer(
        const RemoteSocket& remote_socket,
        Utf8Path static_dir);
    ~ConfigServer();
    bool application_should_exit() const;
    void notify_reload_required();
    void wait_until_reload_required() const;
    virtual std::shared_ptr<ISendSocket> try_receive(std::ostream& ostr) override;
private:
    void handle_session(boost::asio::ip::tcp::socket socket);
    Utf8Path static_dir_;
    std::string cert_hash_;
    bool reload_required_;
    mutable std::mutex mutex_;
    mutable std::condition_variable cv_;
    mutable FastMutex receive_mutex_;
    std::list<std::shared_ptr<IDatagramNode>> websocket_nodes_;
    std::optional<DanglingList<IDatagramNode>> websocket_nodes_in_progress_;
    JThread http_thread_;
};

}
