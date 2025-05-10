#include "Attached_Wheel.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>

using namespace Mlib;

AttachedWheel::AttachedWheel(
    const RigidBodyPulses& vehicle,
    RigidBodyPulses& wheel,
    const FixedArray<float, 3>& vertical_line)
    : vehicle_{ vehicle }
    , wheel_{ wheel }
    , vertical_line_{ vertical_line }
{}

FixedArray<float, 3> AttachedWheel::velocity_at_position(const FixedArray<ScenePos, 3>& position) const {
    auto vv = vehicle_.velocity_at_position(position);
    vv += vertical_line_ * dot0d(vertical_line_, wheel_.v_com_ - vv);
    return vv;
}

float AttachedWheel::effective_mass(const VectorAtPosition<float, ScenePos, 3>& vp) const {
    return wheel_.effective_mass(vp);
}

void AttachedWheel::integrate_impulse(const VectorAtPosition<float, ScenePos, 3>& J, float extra_w) {
    wheel_.integrate_impulse(J, extra_w);
}
