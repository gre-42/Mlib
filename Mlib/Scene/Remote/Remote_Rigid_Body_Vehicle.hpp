#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Cache_Object_Token.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace Mlib {

class PhysicsScene;
class RigidBodyVehicle;
enum class IoVerbosity;
enum class ObjectLifetimeStatus;
enum class RemoteSceneObjectType: RemoteSceneObjectUnderlyingType;

class RemoteRigidBodyVehicle final: public IIncrementalObject {
public:
    explicit RemoteRigidBodyVehicle(
        IoVerbosity verbosity,
        RemoteSceneObjectType type,
        const RemoteObjectId& remote_object_id,
        nlohmann::json initial,
        std::string node_suffix,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    virtual ~RemoteRigidBodyVehicle() override;
    static DanglingBaseClassPtr<RemoteRigidBodyVehicle> try_create_from_stream(
        RemoteSceneObjectType type,
        PhysicsScene& physics_scene,
        BinaryBitwiseWordsReader& reader,
        RemoteSiteId sender_site_id,
        TransmittedFields transmitted_fields,
        ObjectLifetimeStatus lifetime_status,
        const RemoteObjectId& remote_object_id,
        ProxyObjectsCaches& proxy_objects_caches,
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
        TransmissionHistoryReader& transmission_history_reader) override;
    virtual void write(
        BinaryBitwiseWordsWriter& writer,
        RemoteSiteId receiver_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        ProxyObjectsCaches& proxy_objects_caches,
        TransmissionHistoryWriter& transmission_history_writer) override;

    DanglingBaseClassRef<RigidBodyVehicle> rb();
    const std::string& node_suffix() const;

private:
    IncrementalCacheObjectToken proxy_object_cache_token_;
    RemoteSceneObjectType type_;
    nlohmann::json initial_;
    std::string node_suffix_;
    DanglingBaseClassPtr<RigidBodyVehicle> rb_;
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens rb_on_destroy_;
};

}
