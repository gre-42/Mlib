#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <memory>

namespace Mlib {

class RigidBody;
class RigidBodies;

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
std::shared_ptr<RigidBody> rigid_cuboid(
    RigidBodies& rigid_bodies,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>());

}
