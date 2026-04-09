#include "Tire_Contact_Slip.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

TireContactSlip::TireContactSlip(
    const RigidBodyVehicle& rb,
    size_t tire_id,
    const FixedArray<float, 3>& surface_normal,
    const FixedArray<float, 3>& n3,
    const FixedArray<float, 3>& b0)
    : tv{uninitialized}
    , vv{uninitialized}
    , n3_{n3}
{
    v = rb.get_tire_angular_velocity(tire_id) * rb.get_tire_radius(tire_id);
    tv = n3 * v;

    vv = rb.get_velocity_at_tire_contact(surface_normal, tire_id) - b0;
    vvx = dot0d(vv, n3);
    tvx = dot0d(tv, n3);
}

float TireContactSlip::slip(float parking_brake_velocity) const {
    // slip = ((vehicle velocity=vvx) + (tire velocity=tvx)) / (vehicle velocity=vvx)
    return (vvx + tvx) / std::max(parking_brake_velocity, std::abs(vvx));
}

float TireContactSlip::sin_lateral_slip_angle(float parking_brake_velocity) const
{
    // Reference implementation (sin_lateral_slip_angle):
    // {
    //     float vvx = dot0d(vv, n3_);
    //     auto vvt = vv - n3_ * vvx;
    //     auto vvv = vvt + n3_ * (std::abs(vvx) + std::max(0.f, parking_brake_velocity - std::abs(vvx)));
    //     auto vvn = vvv / std::sqrt(sum(squared(vvv)));
    //     return std::sqrt(std::max(0.f, 1 - squared(dot0d(vvn, n3_))));
    // }
    // Optimized implementation (sin_lateral_slip_angle):
    auto ccc = std::max(0.f, sum(squared(vv)) - squared(vvx));
    auto bbb = squared(std::abs(vvx) + std::max(0.f, parking_brake_velocity - std::abs(vvx)));
    return std::sqrt(std::max(0.f, 1 - bbb / (ccc + bbb)));
}
