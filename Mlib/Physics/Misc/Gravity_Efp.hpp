#pragma once
#include <Mlib/Physics/Interfaces/IExternal_Force_Provider.hpp>

namespace Mlib {

class PhysicsEngine;

class GravityEfp: public IExternalForceProvider {
public:
    explicit GravityEfp(PhysicsEngine& engine);
    ~GravityEfp();
    virtual void increment_external_forces(
        bool burn_in,
        const PhysicsEngineConfig& cfg,
        const StaticWorld& world) override;
private:
    PhysicsEngine& engine_;
};

}
