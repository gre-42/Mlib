#pragma once
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/J_Thread.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace Mlib {

class IDatagramSocket;
struct RemoteSocket;

struct ReceivedMessage {
    std::vector<std::byte> message;
    std::shared_ptr<ISendSocket> reply_socket;
};

class ThreadedDatagramNode final: public IDatagramNode {
public:
    explicit ThreadedDatagramNode(std::shared_ptr<IDatagramSocket> socket);
    virtual ~ThreadedDatagramNode() override;
    virtual void start_receive_thread(size_t max_stored_received_messages) override;
    virtual void bind() override;
    virtual void shutdown() override;
    virtual void send(std::istream& istr) override;
    virtual std::shared_ptr<ISendSocket> try_receive(std::ostream& ostr) override;
private:
    std::shared_ptr<IDatagramSocket> socket_;
    FastMutex message_mutex_;
    std::jthread receive_thread_;
    std::list<ReceivedMessage> messages_received_;
};

}
