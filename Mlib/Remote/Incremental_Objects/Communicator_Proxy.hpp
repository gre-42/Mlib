#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Distributed_System.hpp>
#include <iosfwd>
#include <unordered_set>

namespace Mlib {

class ISendSocket;

class CommunicatorProxy final: public virtual ICommunicatorProxy {
public:
    CommunicatorProxy(
        DanglingBaseClassRef<ISendSocket> socket,
        DanglingBaseClassRef<ISharedObjectFactory> shared_object_factory);
    virtual ~CommunicatorProxy() override;
    virtual void receive_from_home(SharedObjects& objects, std::istream& istr) override;
    virtual void send_home(const SharedObjects& objects, std::iostream& iostr) override;
private:
    std::unordered_set<RemoteObjectId> known_objects;
    DanglingBaseClassRef<ISendSocket> socket_;
    DanglingBaseClassRef<ISharedObjectFactory> shared_object_factory_;
};

}
