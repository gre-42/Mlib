#pragma once
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Sockets/Asio.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace Mlib {

struct ReceivedMessage {
    std::vector<std::byte> message;
    std::unique_ptr<ISendSocket> reply_socket;
};

class UdpNode: public ISendSocket, public IReceiveSocket {
public:
    UdpNode(
        const std::string& ip_address,
        uint16_t port);
    UdpNode(
        std::shared_ptr<boost::asio::io_context> io_context,
        std::shared_ptr<boost::asio::ip::udp::socket> socket,
        boost::asio::ip::udp::endpoint endpoint,
        size_t max_stored_received_messages);
    ~UdpNode();
    void start_receive_thread(size_t max_stored_received_messages);
    void bind();
    void shutdown();
    virtual void send(std::istream& istr) override;
    virtual std::unique_ptr<ISendSocket> try_receive(std::ostream& ostr) override;
private:
    std::shared_ptr<boost::asio::io_context> io_context_;
    std::shared_ptr<boost::asio::ip::udp::socket> socket_;
    boost::asio::ip::udp::endpoint endpoint_;
    FastMutex message_mutex_;
    std::jthread receive_thread_;
    std::list<ReceivedMessage> messages_received_;
};

}
