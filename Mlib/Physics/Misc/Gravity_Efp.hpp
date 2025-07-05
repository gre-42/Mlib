#pragma once
#include <Mlib/Physics/Interfaces/IExternal_Force_Provider.hpp>

namespace Mlib {

struct PhysicsPhase;
class PhysicsEngine;

class GravityEfp: public IExternalForceProvider {
public:
    explicit GravityEfp(PhysicsEngine& engine);
    ~GravityEfp();
    virtual void increment_external_forces(
        const PhysicsEngineConfig& cfg,
        const PhysicsPhase& phase,
        const StaticWorld& world) override;
private:
    PhysicsEngine& engine_;
};

}
