#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>

namespace Mlib {

class ObjectPool;
class PhysicsScene;
class Player;
class SceneVehicle;
enum class IoVerbosity;

class RemotePlayer final: public IIncrementalObject {
public:
    explicit RemotePlayer(
        ObjectPool& object_pool,
        IoVerbosity verbosity,
        const DanglingBaseClassRef<Player>& player,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    ~RemotePlayer();
    static DanglingBaseClassPtr<RemotePlayer> try_create_from_stream(
        ObjectPool& object_pool,
        PhysicsScene& physics_scene,
        std::istream& istr,
        TransmittedFields transmitted_fields,
        IoVerbosity verbosity);
    virtual void read(
        std::istream& istr,
        TransmittedFields transmitted_fields) override;
    virtual void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) override;

private:
    DanglingBaseClassRef<Player> player_;
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    DanglingBaseClassPtr<SceneVehicle> vehicle_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens player_on_destroy_;
    DestructionFunctionsRemovalTokens vehicle_on_destroy_;
};

}
