#pragma once
#include <list>
#include <memory>

namespace Mlib {

struct PhysicsEngineConfig;
struct StaticWorld;

class IExternalForceProvider {
public:
    virtual void increment_external_forces(
        bool burn_in,
        const PhysicsEngineConfig& cfg,
        const StaticWorld& world) = 0;
};

}
