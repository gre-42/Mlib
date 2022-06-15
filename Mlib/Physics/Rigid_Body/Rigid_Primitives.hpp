#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <memory>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class RigidBodyPulses;
struct RigidBodyIntegrator;
class RigidBodyVehicle;
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

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyIntegrator rigid_disk_integrator(
    float mass,
    float radius,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>());

std::shared_ptr<RigidBodyVehicle> rigid_cuboid(
    const std::string& name,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const TransformationMatrix<double, double, 3>* geographic_coordinates = nullptr);

std::shared_ptr<RigidBodyVehicle> rigid_disk(
    const std::string& name,
    float mass,
    float radius,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const TransformationMatrix<double, double, 3>* geographic_coordinates = nullptr);

}
