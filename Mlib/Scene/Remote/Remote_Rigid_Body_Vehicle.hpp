#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <memory>
#include <string>

namespace Mlib {

class PhysicsScene;
class RigidBodyVehicle;
class ObjectPool;

class RemoteRigidBodyVehicle final: public IIncrementalObject {
public:
    explicit RemoteRigidBodyVehicle(
        ObjectPool& object_pool,
        std::string initial,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb);
    ~RemoteRigidBodyVehicle();
    static std::unique_ptr<RemoteRigidBodyVehicle> from_stream(
        ObjectPool& object_pool,
        PhysicsScene& physics_scene,
        std::istream& istr);
    virtual void read(std::istream& istr) override;
    virtual void write(std::ostream& ostr, ObjectCompression compression) override;

private:
    std::string initial_;
    DanglingBaseClassRef<RigidBodyVehicle> rb_;
    DestructionFunctionsRemovalTokens rb_on_destroy_;
};

}
