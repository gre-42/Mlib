#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>

namespace Mlib {

class PhysicsScene;
class Player;
class SceneVehicle;
enum class IoVerbosity;

class RemotePlayer final: public IIncrementalObject {
public:
    explicit RemotePlayer(
        IoVerbosity verbosity,
        const DanglingBaseClassRef<Player>& player,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    ~RemotePlayer();
    static DanglingBaseClassPtr<RemotePlayer> try_create_from_stream(
        PhysicsScene& physics_scene,
        std::istream& istr,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader,
        IoVerbosity verbosity);
    virtual void read(
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override;
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
