#include "Power_To_Force.hpp"

using namespace Mlib;

/**
 * solve([1/2*m*((v+F/m*t)^2-v^2) + 1/2*M*((V-F/M*t)^2-V^2)=P*t, m*v + M*V = m*(v+F/m*t) + M*(V-F/M*t)], F);
 */
void Mlib::power_to_forces_finite_masses(
    FixedArray<float, 3>& F0,
    FixedArray<float, 3>& F1,
    const FixedArray<float, 3>& power3,
    float M,
    float m,
    const FixedArray<float, 3>& V3,
    const FixedArray<float, 3>& v3,
    float dt)
{
    float P_sq = sum(squared(power3));
    if (P_sq < 1e-12) {
        F0 = 0;
        F1 = 0;
        return;
    }
    float P = std::sqrt(P_sq);
    auto n = power3 / P;
    float V = dot0d(V3, n);
    float v = dot0d(v3, n);

    auto q = [](auto x){ return squared(x); };
    float F_sqrt = std::sqrt(q(M * m) * (q(v) + q(V)) - 2 * q(M * m) * V * v + 2 * P * dt * (M * q(m) + q(M) * m) * dt);
    float F_c = M * m * (V - v);
    float d = (M + m) * dt;
    if (V > 0) {
        F1 = (F_c - F_sqrt) / d;
    } else {
        F1 = (F_c + F_sqrt) / d;
    }
    if (v > 0) {
        F0 = (F_c + F_sqrt) / d;
    } else {
        F0 = (F_c - F_sqrt) / d;
    }
}

/**
 * Solve x_new^2 + y^2 = r^2 for x_new.
 * If non such x_new exists, i.e. r < |y|, do nothing (y alone is already too large).
 * If |x_new| > |x|, do nothing (power is low enough to not cause friction).
 */
static float correct_x(float x, float y, float r, float safety_factor = 0.99) {
    if (r < std::abs(y)) {
        return x;
    }
    float xa = std::sqrt(squared(r) - squared(y));
    if (std::abs(xa) > std::abs(x)) {
        return x;
    }
    return sign(x) * xa * safety_factor;
}

/**
 * solve(1/2*m*((v+F/m*t)^2-v^2) = P*t, F);
 * 
 * P == NAN => brake
 */
Mlib::FixedArray<float, 3> Mlib::power_to_forces_infinite_mass(
    float break_accel,
    float tangential_accel,
    float hand_break_velocity,
    float max_stiction_force,
    float max_friction_force,
    float friction_force_multiplier,
    float max_velocity,
    const FixedArray<float, 3>& n3,
    float P,
    float m,
    const FixedArray<float, 3>& v3,
    float dt,
    float tire_contact,
    bool avoid_burnout)
{
    float v = dot0d(v3, n3);

    FixedArray<float, 3> sn3T;
    {
        FixedArray<float, 3> v3T = v3 - n3 * v;
        if (float vT_sq = sum(squared(v3T)); vT_sq < 1e-12) {
            sn3T = 0;
        } else {
            sn3T = v3T / (std::sqrt(vT_sq) + 1.f);
        }
    }

    if (!std::isnan(P) && (std::abs(v) > std::abs(max_velocity)) && (sign(P) * v > 0)) {
        P = 0;
    }
    Mlib::FixedArray<float, 3> res;
    // if (!std::isnan(P) && !(sign(P) * v < 0 && std::abs(v) > hand_break_velocity) && !(P == 0 && std::abs(v) < roll_velocity)) {
    if (!std::isnan(P) && (sign(P) * v > 0 || ((P != 0) == (std::abs(v) < hand_break_velocity)))) {
        float F_sqrt = sign(P) * std::sqrt(squared(m * v) + 2 * std::abs(P) * m * dt);
        float F_c = (P != 0) * (-m * v);
        float x = (F_c + F_sqrt) / dt;
        float y = tangential_accel * m;
        if (avoid_burnout) {
            x = correct_x(
                x,
                y * std::sqrt(sum(squared(sn3T))),
                max_stiction_force);
        }
        res = x * n3 - y * sn3T;
    } else {
        FixedArray<float, 3> sn3 = n3 * v / (std::abs(v) + 1.f);
        float x = break_accel * m;
        float y = tangential_accel * m;
        if (avoid_burnout) {
            x = correct_x(
                x * std::sqrt(sum(squared(sn3))),
                y * std::sqrt(sum(squared(sn3T))),
                max_stiction_force);
        }
        res = -x * sn3 - y * sn3T;
    }
    if (float len2 = sum(squared(res)); len2 > squared(max_stiction_force)) {
        if (len2 * squared(friction_force_multiplier) < squared(max_friction_force)) {
            res *= friction_force_multiplier;
        } else {
            res *= max_friction_force / std::sqrt(len2);
        }
    }
    return res * tire_contact;
}

// Mlib::FixedArray<float, 3> F;
// if (sign(P) * v >= 0) {
//     float F_sqrt = sign(P) * std::sqrt(squared(m * v) + 2 * std::abs(P) * m * dt);
//     float F_c = (P != 0) * (-m * v);
//     F = n3 * (F_c + F_sqrt) / dt - tangential_accel * m * sn3T;
// } else {
//     FixedArray<float, 3> sn3 = n3 * P / (std::abs(P) + float(1e3));
//     F = break_accel * m * sn3 - tangential_accel * m * sn3T;
// }
// float F2 = sum(squared(F));
// float max_F = 5e4;
// float slide_F = 1e4;
// if (F2 > squared(max_F)) {
//     F *= slide_F / std::sqrt(F2);
// }
// return F;

/**
 *
 * 1/2 * m * ||v + F / m * t||^2 - 1/2 * m * ||v||^2 = 2 * P * t
 * alpha * d := F / m * t
 * ||v + alpha * d||^2 = 2 * P * t / m + ||v||^2
 * vTv + 2 * alpha * vTd + alpha^2 * dTd = 2 * P * t / m + vTv
 *
 * solve(vTv + 2 * alpha * vTd + alpha^2 * dTd = 2 * P * t / m + vTv, alpha);
 * d -> n => dTd = 1
 */
Mlib::FixedArray<float, 3> power_to_forces_infinite_mass_3D(
    float break_accel,
    const FixedArray<float, 3>& power3,
    float m,
    const FixedArray<float, 3>& v3,
    float dt)
{
    float P_sq = sum(squared(power3));
    if (P_sq < 1e-12) {
        return fixed_zeros<float, 3>();
    }
    float P = std::sqrt(P_sq);
    FixedArray<float, 3> n = power3 / P;

    float vTn = dot0d(v3, n);

    if (vTn > 0) {
        float alpha_sqrt = std::sqrt(std::abs(squared(m * vTn) + 2 * P * m * dt));
        float alpha_c = -m * vTn;
        float alpha0 = (alpha_c + alpha_sqrt) / m;
        float alpha1 = (alpha_c - alpha_sqrt) / m;
        float ab = std::min(alpha0, alpha1);
        float at = std::max(alpha0, alpha1);
        if (ab > 0) {
            return ab * n * m / dt;
        } else {
            return at * n * m / dt;
        }
    } else {
        return break_accel * m * n;
    }

}
