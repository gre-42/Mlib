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
        const DanglingBaseClassRef<ISendSocket>& socket,
        const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
        const DanglingBaseClassRef<IncrementalRemoteObjects>& objects);
    virtual ~IncrementalCommunicatorProxy() override;
    virtual void receive_from_home(std::istream& istr) override;
    virtual void send_home(std::iostream& iostr) override;
private:
    std::unordered_set<RemoteObjectId> known_objects;
    DanglingBaseClassRef<ISendSocket> socket_;
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
};

}
