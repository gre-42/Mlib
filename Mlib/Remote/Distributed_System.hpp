#pragma once
#include <Mlib/Memory/Dangling_List.hpp>
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Remote_Object_Id.hpp>
#include <compare>
#include <cstdint>
#include <iosfwd>

namespace Mlib {

enum class ObjectCompression: uint32_t;
class ISendSocket;
class ICommunicatorProxy;
class ISharedObject;
class IReceiveSocket;

using RemoteCommunicatorId = uint64_t;
using ReceiveSockets = DanglingUnorderedSet<IReceiveSocket>;
using HandshakeProxies = DanglingList<ICommunicatorProxy>;
using CommunicatorProxies = DanglingValueUnorderedMap<RemoteCommunicatorId, ICommunicatorProxy>;
using SharedObjects = DanglingValueUnorderedMap<RemoteObjectId, ISharedObject>;

enum class TransmissionType {
    HANDSHAKE,
    UNICAST,
    MULTICAST
};

class ICommunicatorProxy: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~ICommunicatorProxy() = default;
    virtual void receive_from_home(SharedObjects& objects, std::istream& istr) = 0;
    virtual void send_home(const SharedObjects& objects, std::iostream& iostr) = 0;
};

class ISharedObject: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~ISharedObject() = default;
    virtual void read(std::istream& istr) = 0;
    virtual void write(std::ostream& ostr, ObjectCompression compression) = 0;
};

class ISharedObjectFactory: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassRef<ISharedObject> create_shared_object(std::istream& istr) = 0;
};

class ICommunicatorProxyFactory: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_communicator_proxy(
        DanglingBaseClassRef<ISendSocket> send_socket) = 0;
};

class DistributedSystem {
public:
    DistributedSystem(
        DanglingBaseClassRef<ICommunicatorProxyFactory> communicator_proxy_factory,
        RemoteCommunicatorId location_id);
    ~DistributedSystem();
    void add_receive_socket(DanglingBaseClassRef<IReceiveSocket> socket);
    void add_handshake_socket(DanglingBaseClassRef<ISendSocket> socket);
    void add_object(DanglingBaseClassRef<ISharedObject> object);
    void send_and_receive(TransmissionType transmission_type);
    void print(std::ostream& ostr) const;
private:
    void send(TransmissionType transmission_type);
    void receive();
    ReceiveSockets receive_sockets_;
    RemoteObjectId next_object_id_;
    HandshakeProxies handshake_communicator_proxies_;
    CommunicatorProxies unicast_communicator_proxies_;
    CommunicatorProxies multicast_communicator_proxies_;
    SharedObjects objects_;
    DanglingBaseClassRef<ICommunicatorProxyFactory> communicator_proxy_factory_;
    RemoteCommunicatorId location_id_;
};

std::ostream& operator << (std::ostream& ostr, const DistributedSystem& distributed_system);

}
