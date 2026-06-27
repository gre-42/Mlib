#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>

namespace Mlib {

class PhysicsScene;
class Player;
class SceneVehicle;
enum class IoVerbosity;
enum class ObjectLifetimeStatus;

class RemotePlayer final: public IIncrementalObject {
public:
    explicit RemotePlayer(
        IoVerbosity verbosity,
        const DanglingBaseClassRef<Player>& player,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    ~RemotePlayer();
    static DanglingBaseClassPtr<RemotePlayer> try_create_from_stream(
        PhysicsScene& physics_scene,
        BinaryBitwiseWordsReader& reader,
        TransmittedFields transmitted_fields,
        ObjectLifetimeStatus lifetime_status,
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
    void reset_node();
    DanglingBaseClassRef<Player> player_;
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    DanglingBaseClassPtr<SceneVehicle> vehicle_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens player_on_destroy_;
    DestructionFunctionsRemovalTokens vehicle_on_destroy_;
};

}
