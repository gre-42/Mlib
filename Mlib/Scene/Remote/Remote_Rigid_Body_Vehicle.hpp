#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <memory>
#include <string>

namespace Mlib {

class PhysicsScene;
class RigidBodyVehicle;
enum class IoVerbosity;
enum class RemoteSceneObjectType: uint32_t;

class RemoteRigidBodyVehicle final: public IIncrementalObject {
public:
    explicit RemoteRigidBodyVehicle(
        IoVerbosity verbosity,
        RemoteSceneObjectType type,
        std::string initial,
        std::string node_suffix,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    virtual ~RemoteRigidBodyVehicle() override;
    static DanglingBaseClassPtr<RemoteRigidBodyVehicle> try_create_from_stream(
        RemoteSceneObjectType type,
        PhysicsScene& physics_scene,
        std::istream& istr,
        TransmittedFields transmitted_fields,
        const RemoteObjectId& remote_object_id,
        IoVerbosity verbosity);
    virtual void read(
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override;
    virtual void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) override;

    DanglingBaseClassRef<RigidBodyVehicle> rb();
    const std::string& node_suffix() const;

private:
    RemoteSceneObjectType type_;
    std::string initial_;
    std::string node_suffix_;
    DanglingBaseClassPtr<RigidBodyVehicle> rb_;
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens rb_on_destroy_;
};

}
