#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <memory>

namespace Mlib {

class RigidBodyPulses;
class RigidBodyIntegrator;
class RigidBody;
class RigidBodies;

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyPulses rigid_cuboid_pulses(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>());

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyIntegrator rigid_cuboid_integrator(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>());

std::shared_ptr<RigidBody> rigid_cuboid(
    RigidBodies& rigid_bodies,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>());

}
