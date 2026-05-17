#pragma once
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <emscripten/val.h>
#include <memory>

namespace Mlib {

struct RemoteSocket;

class WebTransportDatagramNode final:
    public IDatagramNode,
    public std::enable_shared_from_this<WebTransportDatagramNode>
{
public:
    explicit WebTransportDatagramNode(const RemoteSocket& socket);
    virtual ~WebTransportDatagramNode() override;
    virtual void start_receive_thread(size_t max_stored_received_messages) override;
    virtual void bind() override;
    virtual void shutdown() override;
    virtual void send(std::istream& istr) override;
    virtual std::shared_ptr<ISendSocket> try_receive(std::ostream& ostr) override;
private:
    RemoteSocket remote_socket_;
    emscripten::val socket_;
};

}
