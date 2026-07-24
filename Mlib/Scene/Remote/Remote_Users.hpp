#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <memory>

namespace Mlib {

class PhysicsScene;
enum class ObjectLifetimeStatus;
enum class IoVerbosity;

class RemoteUsers final: public IIncrementalObject {
public:
    RemoteUsers(
        IoVerbosity verbosity,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        RemoteSiteId site_id);
    ~RemoteUsers();
    static DanglingBaseClassPtr<RemoteUsers> try_create_from_stream(
        PhysicsScene& physics_scene,
        BinaryBitwiseWordsReader& reader,
        TransmittedFields transmitted_fields,
        ObjectLifetimeStatus lifetime_status,
        RemoteSiteId site_id,
        ProxyTasks proxy_tasks,
        TransmissionHistoryReader& transmission_history_reader,
        IoVerbosity verbosity);
    virtual std::string name() const override;
    virtual int32_t priority() const override;
    virtual void read(
        BinaryBitwiseWordsReader& reader,
        RemoteSiteId sender_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        ProxyObjectsCaches& proxy_objects_caches,
        const IncrementalVersionsRead& versions,
        TransmissionHistoryReader& transmission_history_reader) override;
    virtual void write(
        BinaryBitwiseWordsWriter& writer,
        RemoteSiteId receiver_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        ProxyObjectsCaches& proxy_objects_caches,
        const IncrementalVersionsWrite& versions,
        TransmissionHistoryWriter& transmission_history_writer) override;

private:
    void read_data(
        BinaryBitwiseWordsReader& reader,
        TransmittedFields transmitted_fields,
        ProxyTasks proxy_tasks,
        TransmissionHistoryReader& transmission_history_reader);

    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
    RemoteSiteId site_id_;
    DestructionFunctionsRemovalTokens physics_scene_on_destroy_;
};

}
