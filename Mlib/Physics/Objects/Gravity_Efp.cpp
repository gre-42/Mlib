#include "Gravity_Efp.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>

using namespace Mlib;

GravityEfp::GravityEfp(const FixedArray<float, 3>& gravity)
: gravity_{gravity}
{}

void GravityEfp::increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in) {
    for(auto& rb : olist) {
        if (rb->mass() != INFINITY) {
            rb->integrate_gravity(gravity_);
        }
    }
}
