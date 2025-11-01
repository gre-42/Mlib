#pragma once
#include <Mlib/Memory/Dangling_List.hpp>
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <compare>
#include <iosfwd>
#include <memory>

namespace Mlib {

class ISendSocket;
class ICommunicatorProxy;
class IReceiveSocket;

using ReceiveSockets = DanglingUnorderedSet<IReceiveSocket>;
using HandshakeProxies = DanglingList<ICommunicatorProxy>;
using CommunicatorProxyMap = DanglingValueUnorderedMap<RemoteSiteId, ICommunicatorProxy>;

enum class TransmissionType {
    HANDSHAKE,
    UNICAST,
    MULTICAST
};

class ICommunicatorProxy: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~ICommunicatorProxy() = default;
    virtual void set_send_socket(std::shared_ptr<ISendSocket> send_socket) = 0;
    virtual void receive_from_home(std::istream& istr) = 0;
    virtual void send_home(std::iostream& iostr) = 0;
};

class ICommunicatorProxyFactory: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_handshake_proxy(
        std::shared_ptr<ISendSocket> send_socket) = 0;
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_communicator_proxy(
        std::shared_ptr<ISendSocket> send_socket,
        RemoteSiteId home_site_id) = 0;
};

class CommunicatorProxies {
public:
    CommunicatorProxies(
        const DanglingBaseClassRef<ICommunicatorProxyFactory>& communicator_proxy_factory,
        RemoteSiteId site_id);
    ~CommunicatorProxies();
    void add_receive_socket(const DanglingBaseClassRef<IReceiveSocket>& socket);
    void add_handshake_socket(std::shared_ptr<ISendSocket> socket);
    void send_and_receive(TransmissionType transmission_type);
    void print(std::ostream& ostr) const;
private:
    void send(TransmissionType transmission_type);
    void receive();
    ReceiveSockets receive_sockets_;
    HandshakeProxies handshake_communicator_proxies_;
    CommunicatorProxyMap unicast_communicator_proxies_;
    CommunicatorProxyMap multicast_communicator_proxies_;
    DanglingBaseClassRef<ICommunicatorProxyFactory> communicator_proxy_factory_;
    RemoteSiteId site_id_;
};

std::ostream& operator << (std::ostream& ostr, const CommunicatorProxies& distributed_system);

}
