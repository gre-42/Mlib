#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>

namespace Mlib {

class PhysicsScene;
enum class IoVerbosity;

class RemoteSceneObjectFactory final: public IIncrementalObjectFactory {
public:
    explicit RemoteSceneObjectFactory(
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        IoVerbosity verbosity);
    virtual ~RemoteSceneObjectFactory() override;
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(
        BinaryBitwiseWordsReader& reader,
        RemoteSiteId sender_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        ObjectLifetimeStatus lifetime_status,
        ProxyObjectsCaches& proxy_objects_caches,
        const IncrementalVersionsRead& versions,
        TransmissionHistoryReader& transmission_history_reader) override;
private:
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
};

}
