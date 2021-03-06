#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;

class GravityEfp: public ExternalForceProvider {
public:
    explicit GravityEfp(const FixedArray<float, 3>& gravity);
    virtual void increment_external_forces(
        const std::list<std::shared_ptr<RigidBodyVehicle>>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg) override;
private:
    FixedArray<float, 3> gravity_;
};

}
