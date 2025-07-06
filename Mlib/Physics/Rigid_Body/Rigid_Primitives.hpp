#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Physics_Engine/Penetration_Limits_Factory.hpp>
#include <memory>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class RigidBodyPulses;
class RigidBodyVehicle;
class RigidBodies;
template <class T>
class DeleteFromPool;
class ObjectPool;

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyPulses rigid_cuboid_pulses(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& I_rotation = fixed_zeros<float, 3>(),
    const PenetrationLimitsFactory& pl = PenetrationLimitsFactory::inf());

// Source: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
RigidBodyPulses rigid_disk_pulses(
    float mass,
    float radius,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& I_rotation = fixed_zeros<float, 3>(),
    const PenetrationLimitsFactory& pl = PenetrationLimitsFactory::inf());

std::unique_ptr<RigidBodyVehicle, DeleteFromPool<RigidBodyVehicle>> rigid_cuboid(
    ObjectPool& object_pool,
    std::string name,
    std::string asset_id,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& I_rotation = fixed_zeros<float, 3>(),
    const PenetrationLimitsFactory& pl = PenetrationLimitsFactory::inf(),
    const TransformationMatrix<double, double, 3>* geographic_coordinates = nullptr);

std::unique_ptr<RigidBodyVehicle, DeleteFromPool<RigidBodyVehicle>> rigid_disk(
    ObjectPool& object_pool,
    std::string name,
    std::string asset_id,
    float mass,
    float radius,
    const FixedArray<float, 3>& com = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& v = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& w = fixed_zeros<float, 3>(),
    const FixedArray<float, 3>& I_rotation = fixed_zeros<float, 3>(),
    const PenetrationLimitsFactory& pl = PenetrationLimitsFactory::inf(),
    const TransformationMatrix<double, double, 3>* geographic_coordinates = nullptr);

}
