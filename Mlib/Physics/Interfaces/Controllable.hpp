#pragma once

namespace Mlib {

struct PhysicsEngineConfig;

class Controllable {
public:
    virtual void notify_reset(bool burn_in, const PhysicsEngineConfig& cfg) = 0;
};

}
