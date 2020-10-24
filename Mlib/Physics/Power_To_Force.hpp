#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

/**
 * solve([1/2*m*((v+F/m*t)^2-v^2) + 1/2*M*((V-F/M*t)^2-V^2)=P*t, m*v + M*V = m*(v+F/m*t) + M*(V-F/M*t)], F);
 */
void power_to_forces_finite_masses(
    FixedArray<float, 3>& F0,
    FixedArray<float, 3>& F1,
    const FixedArray<float, 3>& power3,
    float M,
    float m,
    const FixedArray<float, 3>& V3,
    const FixedArray<float, 3>& v3,
    float dt);

/**
 * solve(1/2*m*((v+F/m*t)^2-v^2) = P*t, F);
 * 
 * P == NAN => brake
 */
FixedArray<float, 3> power_to_force_infinite_mass(
    float break_accel,
    float hand_break_velocity,
    float max_stiction_force,
    float friction_force,
    float max_velocity,
    const FixedArray<float, 3>& n3,
    float P,
    float m,
    const FixedArray<float, 3>& v3,
    float dt,
    float alpha0,
    bool avoid_burnout);

Mlib::FixedArray<float, 3> friction_force_infinite_mass(
    float max_stiction_force,
    float friction_force,
    float m,
    const FixedArray<float, 3>& v3,
    float alpha0);

}
