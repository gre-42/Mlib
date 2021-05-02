#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <memory>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;
class RigidBodyPulses;
struct RigidBodyIntegrator;
class RigidBody;
class RigidBodies;

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyPulses rigid_cuboid_pulses(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>());

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyIntegrator rigid_cuboid_integrator(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>());

std::shared_ptr<RigidBody> rigid_cuboid(
    RigidBodies& rigid_bodies,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const TransformationMatrix<double, 3>* geographic_coordinates = nullptr,
    const std::string& name = "");

}
