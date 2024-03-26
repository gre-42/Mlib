#pragma once

namespace Mlib {

struct PhysicsEngineConfig;

class IControllable {
public:
    virtual void notify_reset(bool burn_in, const PhysicsEngineConfig& cfg) = 0;
};

}
