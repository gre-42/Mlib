#pragma once
#include <list>
#include <memory>

namespace Mlib {

struct PhysicsEngineConfig;
struct PhysicsPhase;
struct StaticWorld;

class IExternalForceProvider {
public:
    virtual void increment_external_forces(
        const PhysicsEngineConfig& cfg,
        const PhysicsPhase& phase,
        const StaticWorld& world) = 0;
};

}
