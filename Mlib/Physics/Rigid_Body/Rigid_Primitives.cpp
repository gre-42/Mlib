#include "Rigid_Primitives.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

FixedArray<float, 3, 3> permuted_diagonal_matrix(
    const FixedArray<float, 3, 3>& I,
    const FixedArray<float, 3>& rotation)
{
    if (all(rotation == 0.f)) {
        return I;
    }
    if (any(abs(I) == INFINITY)) {
        THROW_OR_ABORT("INF not supported yet for permuted matrices");
    }
    auto R = tait_bryan_angles_2_matrix(rotation);
    auto Rr = R.applied([](float v){ return std::round(v); });
    if (any(abs(Rr - R) > 1e-12f)) {
        THROW_OR_ABORT("Rotation cannot be rounded without error");
    }
    auto m = dot2d(R, dot2d(I, R.T()));
    bool is_diagonal = all(abs((1.f - fixed_identity_array<float, 3>()) * m) == 0.f);
    if (!is_diagonal) {
        THROW_OR_ABORT("Permuted matrix is not diagonal");
    }
    return m;
}

RigidBodyPulses Mlib::rigid_cuboid_pulses(
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w,
    const FixedArray<float, 3>& I_rotation,
    const PenetrationLimitsFactory& pl)
{
    // From: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
    auto I = FixedArray<float, 3, 3>::init(
        1.f / 12.f * mass * (squared(size(1)) + squared(size(2))), 0.f, 0.f,
        0.f, 1.f / 12.f * mass * (squared(size(0)) + squared(size(2))), 0.f,
        0.f, 0.f, 1.f / 12.f * mass * (squared(size(0)) + squared(size(1))));

    return RigidBodyPulses{
        mass,
        permuted_diagonal_matrix(I, I_rotation),    // I
        com,                                        // com
        v,                                          // v
        w,                                          // w
        fixed_nans<ScenePos, 3>(),                  // position
        fixed_zeros<float, 3>(),                    // rotation (not NAN to pass rogridues angle assertion)
        true,                                       // I_is_diagonal
        pl
    };
}

RigidBodyPulses Mlib::rigid_disk_pulses(
    float mass,
    float radius,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w,
    const FixedArray<float, 3>& I_rotation,
    const PenetrationLimitsFactory& pl)
{
    // From: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
    auto I = FixedArray<float, 3, 3>::init(
        1.f / 4.f * mass * squared(radius), 0.f, 0.f,
        0.f, 1.f / 4.f * mass * squared(radius), 0.f,
        0.f, 0.f, 1.f / 2.f * mass * squared(radius));

    return RigidBodyPulses{
        mass,
        permuted_diagonal_matrix(I, I_rotation),    // I
        com,                                        // com
        v,                                          // v
        w,                                          // w
        fixed_nans<ScenePos, 3>(),                  // position
        fixed_zeros<float, 3>(),                    // rotation (not NAN to pass rogridues angle assertion)
        true,                                       // I_is_diagonal
        pl,
    };
}

std::unique_ptr<RigidBodyVehicle, DeleteFromPool<RigidBodyVehicle>> Mlib::rigid_cuboid(
    std::string name,
    std::string asset_id,
    float mass,
    const FixedArray<float, 3>& size,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w,
    const FixedArray<float, 3>& I_rotation,
    const PenetrationLimitsFactory& pl,
    const TransformationMatrix<double, double, 3>* geographic_coordinates)
{
    return global_object_pool.create_unique<RigidBodyVehicle>(
        CURRENT_SOURCE_LOCATION,
        rigid_cuboid_pulses(mass, size, com, v, w, I_rotation, pl),
        std::move(name),
        std::move(asset_id),
        geographic_coordinates);
}

std::unique_ptr<RigidBodyVehicle, DeleteFromPool<RigidBodyVehicle>> Mlib::rigid_disk(
    std::string name,
    std::string asset_id,
    float mass,
    float radius,
    const FixedArray<float, 3>& com,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w,
    const FixedArray<float, 3>& I_rotation,
    const PenetrationLimitsFactory& pl,
    const TransformationMatrix<double, double, 3>* geographic_coordinates)
{
    return global_object_pool.create_unique<RigidBodyVehicle>(
        CURRENT_SOURCE_LOCATION,
        rigid_disk_pulses(mass, radius, com, v, w, I_rotation, pl),
        std::move(name),
        std::move(asset_id),
        geographic_coordinates);
}
