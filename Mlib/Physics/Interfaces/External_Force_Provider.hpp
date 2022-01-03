#pragma once
#include <list>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
struct PhysicsEngineConfig;

class ExternalForceProvider {
public:
    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBodyVehicle>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) = 0;
};

}
