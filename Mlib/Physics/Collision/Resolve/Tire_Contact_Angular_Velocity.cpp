#include "Tire_Contact_Angular_Velocity.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

TireContactAngularVelocity::TireContactAngularVelocity(
    const RigidBodyVehicle& rb,
    size_t tire_id,
    const FixedArray<float, 3>& surface_normal,
    const FixedArray<float, 3>& v_street)
    : rb_{rb}
    , tire_id_{tire_id}
{
    vv = rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id) * rb.get_tire_radius(tire_id);
}

float TireContactAngularVelocity::tire_angular_velocity(float slip, float parking_brake_velocity) const
{
    float y = std::max(parking_brake_velocity, std::abs(vv));
    return (slip * y - vv) / rb_.get_tire_radius(tire_id_);
}
