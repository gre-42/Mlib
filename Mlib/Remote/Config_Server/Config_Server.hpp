#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <condition_variable>
#include <mutex>

namespace Mlib {

struct RemoteSocket;

class ConfigServer {
public:
    explicit ConfigServer(
        const RemoteSocket& remote_socket,
        Utf8Path static_dir);
    ~ConfigServer();
    bool application_should_exit() const;
    void notify_reload_required();
    void wait_until_reload_required() const;
private:
    void handle_session(boost::asio::ip::tcp::socket socket);
    Utf8Path static_dir_;
    bool reload_required_;
    mutable std::mutex mutex_;
    mutable std::condition_variable cv_;
    JThread thread_;
};

}
