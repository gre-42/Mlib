#include "Handle_Tire_Triangle_Intersection.hpp"
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Power_To_Force.hpp>

using namespace Mlib;

void accelerate_positive(
    RigidBody& rb,
    float power,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id)
{
    float v_max = 400.f / 3.6f;
    // r = v / w
    float f = 1;
    if (float vc = sum(squared(rb.rbi_.v_)); vc > 1e-12) {
        if (float vt = sum(squared(rb.get_velocity_at_tire(tire_id))); vt > 1e-12) {
            f = std::clamp<float>(std::sqrt(vt / vc), 1e-1, 1e1);
        }
    }
    float vv;
    for (vv = 0; vv >= -v_max; vv -= 0.1) {
        FixedArray<float, 3> tf = friction_force_infinite_mass(
            cfg.stiction_coefficient * force_n1,
            cfg.friction_coefficient * force_n1,
            v3 + n3 * vv,
            cfg.alpha0);
        if (dot0d(tf, n3) > std::abs(power / vv) * f) {
            break;
        }
    }
    rb.set_tire_angular_velocity(tire_id, vv / rb.get_tire_radius(tire_id));
}

void accelerate_negative(
    RigidBody& rb,
    float power,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id)
{
    float v_max = 400.f / 3.6f;
    // r = v / w
    float f = 1;
    if (float vc = sum(squared(rb.rbi_.v_)); vc > 1e-12) {
        if (float vt = sum(squared(rb.get_velocity_at_tire(tire_id))); vt > 1e-12) {
            f = std::clamp<float>(std::sqrt(vt / vc), 1e-1, 1e1);
        }
    }
    float vv;
    for (vv = 0; vv <= v_max; vv += 0.1) {
        FixedArray<float, 3> tf = friction_force_infinite_mass(
            cfg.stiction_coefficient * force_n1,
            cfg.friction_coefficient * force_n1,
            v3 + n3 * vv,
            cfg.alpha0);
        if (-dot0d(tf, n3) > std::abs(power / vv) * f) {
            break;
        }
    }
    rb.set_tire_angular_velocity(tire_id, vv / rb.get_tire_radius(tire_id));
}

void break_positive(
    RigidBody& rb,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id)
{
    float w = rb.get_angular_velocity_at_tire(tire_id);
    rb.set_tire_angular_velocity(
        tire_id,
        std::max(w - 10.f, 0.f));
}

void break_negative(
    RigidBody& rb,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id)
{
    float w = rb.get_angular_velocity_at_tire(tire_id);
    rb.set_tire_angular_velocity(
        tire_id,
        std::min(w + 10.f, 0.f));
}

void idle(RigidBody& rb, size_t tire_id) {
    rb.set_tire_angular_velocity(tire_id, rb.get_angular_velocity_at_tire(tire_id));
}

FixedArray<float, 3> Mlib::handle_tire_triangle_intersection(
    RigidBody& rb,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id)
{
    PowerIntent P = rb.consume_tire_surface_power(tire_id);
    // F = W / s = W / v / t = P / v
    if (!std::isnan(P.power)) {
        // std::cerr << "dx " << dx << std::endl;
        bool slipping = false;
        if ((P.power != 0) && !slipping) {
            float v = dot0d(rb.rbi_.v_, n3);
            if (sign(P.power) != sign(v) && std::abs(v) > cfg.hand_break_velocity) {
                if (P.power > 0) {
                    break_positive(rb, v3, n3, force_n1, cfg, tire_id);
                } else if (P.power < 0) {
                    break_negative(rb, v3, n3, force_n1, cfg, tire_id);
                }
            } else if (P.power > 0) {
                if (P.type == PowerIntentType::BREAK_OR_IDLE) {
                    idle(rb, tire_id);
                } else {
                    accelerate_positive(rb, P.power, v3, n3, force_n1, cfg, tire_id);
                }
            } else if (P.power < 0) {
                if (P.type == PowerIntentType::BREAK_OR_IDLE) {
                    idle(rb, tire_id);
                } else {
                    accelerate_negative(rb, P.power, v3, n3, force_n1, cfg, tire_id);
                }
            }
        } else if (P.power == 0) {
            idle(rb, tire_id);
        }
    }
    float v1 = rb.get_tire_angular_velocity(tire_id) * rb.get_tire_radius(tire_id);
    return friction_force_infinite_mass(
        cfg.stiction_coefficient * force_n1,
        cfg.friction_coefficient * force_n1,
        v3 + n3 * v1,
        cfg.alpha0);
}
