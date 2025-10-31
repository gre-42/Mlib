#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <iosfwd>
#include <unordered_set>

namespace Mlib {

class ISendSocket;
class IIncrementalObjectFactory;

class IncrementalCommunicatorProxy final: public virtual ICommunicatorProxy {
public:
    IncrementalCommunicatorProxy(
        std::shared_ptr<ISendSocket> send_socket,
        const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
        const DanglingBaseClassRef<IncrementalRemoteObjects>& objects);
    virtual ~IncrementalCommunicatorProxy() override;
    virtual void set_send_socket(std::shared_ptr<ISendSocket> send_socket) override;
    virtual void receive_from_home(std::istream& istr) override;
    virtual void send_home(std::iostream& iostr) override;
private:
    std::unordered_set<RemoteObjectId> known_objects;
    std::shared_ptr<ISendSocket> send_socket_;
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
};

}
