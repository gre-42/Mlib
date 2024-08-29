#pragma once
#include <list>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
struct PhysicsEngineConfig;
struct StaticWorld;

class IExternalForceProvider {
public:
    virtual void increment_external_forces(
        const std::list<RigidBodyVehicle*>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg,
        const StaticWorld& world) = 0;
};

}
