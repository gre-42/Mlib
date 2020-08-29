#include "Rigid_Primitives.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>

using namespace Mlib;

std::shared_ptr<RigidBody> Mlib::rigid_cuboid(
    RigidBodies& rigid_bodies,
    float mass,
    const FixedArray<float, 3>& size)
{
    return std::make_shared<RigidBody>(
        rigid_bodies,
        mass,
        fixed_zeros<float, 3>(), // L
        FixedArray<float, 3, 3>{
            1.f / 12 * mass * (squared(size(1)) + squared(size(2))), 0, 0,
            0, 1.f / 12 * mass * (squared(size(0)) + squared(size(2))), 0,
            0, 0, 1.f / 12 * mass * (squared(size(0)) + squared(size(1)))},
        fixed_zeros<float, 3>(),  // com
        fixed_zeros<float, 3>(),  // v
        fixed_zeros<float, 3>(),  // w
        fixed_zeros<float, 3>(),  // M
        fixed_nans<float, 3>(),   // position
        fixed_nans<float, 3>(),   // rotation
        true);                    // I_is_diagonal
}
