#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <memory>
#include <string>

namespace Mlib {

class PhysicsScene;
class RigidBodyVehicle;
enum class IoVerbosity;

class RemoteRigidBodyVehicle final: public IIncrementalObject {
public:
    explicit RemoteRigidBodyVehicle(
        ObjectPool& object_pool,
        IoVerbosity verbosity,
        std::string initial,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb);
    ~RemoteRigidBodyVehicle();
    static DanglingBaseClassPtr<RemoteRigidBodyVehicle> try_create_from_stream(
        ObjectPool& object_pool,
        PhysicsScene& physics_scene,
        std::istream& istr,
        IoVerbosity verbosity);
    virtual void read(std::istream& istr) override;
    virtual void write(std::ostream& ostr, ObjectCompression compression) override;

    DanglingBaseClassRef<RigidBodyVehicle> rb();

private:
    std::string initial_;
    DanglingBaseClassRef<RigidBodyVehicle> rb_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens rb_on_destroy_;
};

}
