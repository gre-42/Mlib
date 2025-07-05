#pragma once

namespace Mlib {

struct PhysicsEngineConfig;
struct PhysicsPhase;

class IControllable {
public:
    virtual void notify_reset(const PhysicsEngineConfig& cfg, const PhysicsPhase& phase) = 0;
};

}
