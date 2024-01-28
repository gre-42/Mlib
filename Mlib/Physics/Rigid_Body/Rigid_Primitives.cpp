#include "Rigid_Primitives.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

RigidBodyPulses Mlib::rigid_cuboid_pulses(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w)
{
    // From: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
    auto I = FixedArray<float, 3, 3>::init(
        1.f / 12.f * mass * (squared(size(1)) + squared(size(2))), 0.f, 0.f,
        0.f, 1.f / 12.f * mass * (squared(size(0)) + squared(size(2))), 0.f,
        0.f, 0.f, 1.f / 12.f * mass * (squared(size(0)) + squared(size(1))));
    
    return RigidBodyPulses{
        mass,
        I,                                  // I
        com,                                // com
        v,                                  // v
        w,                                  // w
        fixed_nans<double, 3>(),            // position
        fixed_zeros<float, 3>(),            // rotation (not NAN to pass rogridues angle assertion)
        true                                // I_is_diagonal
    };
}

RigidBodyPulses Mlib::rigid_disk_pulses(
    float mass,
    float radius,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w)
{
    // From: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
    auto I = FixedArray<float, 3, 3>::init(
        1.f / 4.f * mass * squared(radius), 0.f, 0.f,
        0.f, 1.f / 4.f * mass * squared(radius), 0.f,
        0.f, 0.f, 1.f / 2.f * mass * squared(radius));
    
    return RigidBodyPulses{
        mass,
        I,                                  // I
        com,                                // com
        v,                                  // v
        w,                                  // w
        fixed_nans<double, 3>(),            // position
        fixed_zeros<float, 3>(),            // rotation (not NAN to pass rogridues angle assertion)
        true                                // I_is_diagonal
    };
}

std::unique_ptr<RigidBodyVehicle> Mlib::rigid_cuboid(
    const std::string& name,
    const std::string& asset_id,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w,
    const TransformationMatrix<double, double, 3>* geographic_coordinates)
{
    return std::make_unique<RigidBodyVehicle>(
        rigid_cuboid_pulses(mass, size, com, v, w),
        name,
        asset_id,
        geographic_coordinates);
}

std::unique_ptr<RigidBodyVehicle> Mlib::rigid_disk(
    const std::string& name,
    const std::string& asset_id,
    float mass,
    float radius,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w,
    const TransformationMatrix<double, double, 3>* geographic_coordinates)
{
    return std::make_unique<RigidBodyVehicle>(
        rigid_disk_pulses(mass, radius, com, v, w),
        name,
        asset_id,
        geographic_coordinates);
}
