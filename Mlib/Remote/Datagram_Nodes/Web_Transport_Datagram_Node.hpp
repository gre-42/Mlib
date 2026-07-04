#pragma once
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <cstddef>
#include <emscripten/val.h>
#include <memory>
#include <vector>

namespace Mlib {

struct RemoteSocket;

class WebTransportDatagramNode final:
    public IDatagramNode,
    public std::enable_shared_from_this<WebTransportDatagramNode>
{
    struct ConstructorKey { explicit ConstructorKey() = default; };

    WebTransportDatagramNode(const WebTransportDatagramNode&) = delete;
    WebTransportDatagramNode& operator = (const WebTransportDatagramNode&) = delete;
public:
    explicit WebTransportDatagramNode(
        ConstructorKey,
        const RemoteSocket& socket,
        std::vector<std::byte> cert_hash,
        std::string remote_secret);
    static std::shared_ptr<WebTransportDatagramNode> create(
        const RemoteSocket& socket,
        std::vector<std::byte> cert_hash,
        std::string remote_secret);
    virtual ~WebTransportDatagramNode() override;
    virtual void start_receive_thread(uint32_t max_stored_received_messages) override;
    virtual void bind() override;
    virtual void send(std::istream& istr) override;
    virtual std::shared_ptr<ISendSocket> try_receive(
        std::ostream& ostr,
        NetworkTransmissionStatus& transmission_status) override;
private:
    RemoteSocket remote_socket_;
    int socket_handle_;
    std::vector<std::byte> cert_hash_;
    std::string remote_secret_;
};

}
