#include "Gravity_Efp.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>

using namespace Mlib;

GravityEfp::GravityEfp(const FixedArray<float, 3>& gravity)
: gravity_{gravity}
{}

void GravityEfp::increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) {
    for (auto& rb : olist) {
        if (rb->mass() != INFINITY) {
            if (cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                rb->integrate_gravity(gravity_);
            } else if (cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                rb->rbi_.rbp_.integrate_gravity(gravity_, cfg.dt / cfg.oversampling);
            } else {
                throw std::runtime_error("Unknown resolve collision type in increment_external_forces");
            }
        }
    }
}
