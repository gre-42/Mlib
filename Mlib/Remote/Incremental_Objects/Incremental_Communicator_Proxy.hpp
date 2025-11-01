#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <iosfwd>
#include <unordered_set>

namespace Mlib {

class ISendSocket;
class IIncrementalObjectFactory;
enum class IoVerbosity;

enum class ProxyTasks {
    NONE = 0,
    SEND_LOCAL = 1 << 0,
    SEND_REMOTE = 1 << 1
};

inline bool any(ProxyTasks tasks) {
    return tasks != ProxyTasks::NONE;
}

inline ProxyTasks operator & (ProxyTasks a, ProxyTasks b) {
    return (ProxyTasks)((int)a & (int)b);
}

inline ProxyTasks operator | (ProxyTasks a, ProxyTasks b) {
    return (ProxyTasks)((int)a | (int)b);
}

inline ProxyTasks& operator |= (ProxyTasks& a, ProxyTasks b) {
    (int&)a |= (int)b;
    return a;
}

class IncrementalCommunicatorProxy final: public virtual ICommunicatorProxy {
public:
    IncrementalCommunicatorProxy(
        std::shared_ptr<ISendSocket> send_socket,
        const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
        const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
        IoVerbosity verbosity,
        ProxyTasks tasks,
        RemoteSiteId home_site_id);
    virtual ~IncrementalCommunicatorProxy() override;
    virtual void set_send_socket(std::shared_ptr<ISendSocket> send_socket) override;
    virtual void receive_from_home(std::istream& istr) override;
    virtual void send_home(std::iostream& iostr) override;
private:
    std::unordered_set<RemoteObjectId> known_objects;
    std::shared_ptr<ISendSocket> send_socket_;
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
    IoVerbosity verbosity_;
    ProxyTasks tasks_;
    RemoteSiteId home_site_id_;
};

}
