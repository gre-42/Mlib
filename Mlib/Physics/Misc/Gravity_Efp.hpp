#pragma once
#include <Mlib/Physics/Interfaces/IExternal_Force_Provider.hpp>

namespace Mlib {

class RigidBodyVehicle;

class GravityEfp: public IExternalForceProvider {
public:
    GravityEfp();
    ~GravityEfp();
    virtual void increment_external_forces(
        const std::list<RigidBodyVehicle*>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg,
        const StaticWorld& world) override;
};

}
