#include "Gravity_Efp.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

GravityEfp::GravityEfp(const FixedArray<float, 3>& gravity)
: gravity_{gravity}
{}

void GravityEfp::increment_external_forces(
    const std::list<std::shared_ptr<RigidBodyVehicle>>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg)
{
    for (auto& rb : olist) {
        if (rb->feels_gravity_ && (rb->mass() != INFINITY)) {
            rb->rbi_.rbp_.integrate_gravity(gravity_, cfg.dt / cfg.oversampling);
        }
    }
}
