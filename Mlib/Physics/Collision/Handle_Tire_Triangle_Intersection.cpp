#include "Handle_Tire_Triangle_Intersection.hpp"
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>

using namespace Mlib;

void optimal_angular_velocity_positive(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& w,
    float* v = nullptr)
{
    float vv = rb.get_angular_velocity_at_tire(surface_normal, street_velocity, tire_id) * rb.get_tire_radius(tire_id);
    float y = std::max(cfg.hand_brake_velocity, std::abs(vv));
    float m = rb.tires_.at(tire_id).magic_formula.longitudinal().argmax;
    w = (m * y - vv) / rb.get_tire_radius(tire_id);
    if (v != nullptr) {
        *v = vv;
    }
}

void optimal_angular_velocity_negative(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& w,
    float* v = nullptr)
{
    float vv = rb.get_angular_velocity_at_tire(surface_normal, street_velocity, tire_id) * rb.get_tire_radius(tire_id);
    float y = std::max(cfg.hand_brake_velocity, std::abs(vv));
    float m = -rb.tires_.at(tire_id).magic_formula.longitudinal().argmax;
    w = (m * y - vv) / rb.get_tire_radius(tire_id);
    if (v != nullptr) {
        *v = vv;
    }
}

void accelerate_positive(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    float power,
    const FixedArray<float, 3>& vc,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    // r = v / w
    float u = 1;
    if (float vc2 = sum(squared(vc)); vc2 > 1e-12) {
        if (-v0 > 1e-12) {
            u = std::clamp<float>(-v0 / std::sqrt(vc2), 1e-1, 1e1);
        }
    }
    float w;
    float v;
    optimal_angular_velocity_positive(rb, street_velocity, surface_normal, cfg, tire_id, w, &v);
    rb.set_tire_angular_velocity(tire_id, -w);
    force_min = u * power / std::min(-0.001f, v);
    force_max = 0;
}

void accelerate_negative(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    float power,
    const FixedArray<float, 3>& vc,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    // r = v / w
    float u = 1;
    if (float vc2 = sum(squared(vc)); vc2 > 1e-12) {
        if (v0 > 1e-12) {
            u = std::clamp<float>(v0 / std::sqrt(vc2), 1e-1, 1e1);
        }
    }
    float w;
    float v;
    optimal_angular_velocity_negative(rb, street_velocity, surface_normal, cfg, tire_id, w, &v);
    rb.set_tire_angular_velocity(tire_id, -w);
    force_min = 0;
    force_max = -u * power / std::max(0.001f, -v);
}

void break_positive(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float w;
    optimal_angular_velocity_positive(rb, street_velocity, surface_normal, cfg, tire_id, w);
    if (sign(rb.get_tire_angular_velocity(tire_id)) != sign(-w)) {
        rb.set_tire_angular_velocity(tire_id, 0);
    } else {
        rb.set_tire_angular_velocity(tire_id, -w);
    }
    force_min = -rb.tires_.at(tire_id).break_force;
    force_max = 0;
    // FixedArray<float, 3> tf0 = friction_force_infinite_mass(
    //     cfg.stiction_coefficient * force_n1,
    //     cfg.friction_coefficient * force_n1,
    //     vc,
    //     cfg.alpha0);
    // float f0 = dot0d(tf0, z3);
    // float v_max = 400.f / 3.6f;
    // float vv;
    // for (vv = v_max; vv >= 0; vv -= 0.1) {
    //     FixedArray<float, 3> tf = friction_force_infinite_mass(
    //         cfg.stiction_coefficient * force_n1,
    //         cfg.friction_coefficient * force_n1,
    //         vc + n3 * vv,
    //         cfg.alpha0);
    //     if (dot0d(tf, z3) > f0 * 0.5f) {
    //         break;
    //     }
    // }
    // rb.set_tire_angular_velocity(tire_id, vv / rb.get_tire_radius(tire_id));
}

void break_negative(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float w;
    optimal_angular_velocity_negative(rb, street_velocity, surface_normal, cfg, tire_id, w);
    if (sign(rb.get_tire_angular_velocity(tire_id)) != sign(-w)) {
        rb.set_tire_angular_velocity(tire_id, 0);
    } else {
        rb.set_tire_angular_velocity(tire_id, -w);
    }
    force_min = 0;
    force_max = rb.tires_.at(tire_id).break_force;
    // FixedArray<float, 3> tf0 = friction_force_infinite_mass(
    //     cfg.stiction_coefficient * force_n1,
    //     cfg.friction_coefficient * force_n1,
    //     vc,
    //     cfg.alpha0);
    // float f0 = dot0d(tf0, n3);
    // float v_max = 400.f / 3.6f;
    // float vv;
    // for (vv = -v_max; vv <= 0; vv += 0.1) {
    //     FixedArray<float, 3> tf = friction_force_infinite_mass(
    //         cfg.stiction_coefficient * force_n1,
    //         cfg.friction_coefficient * force_n1,
    //         vc + n3 * vv,
    //         cfg.alpha0);
    //     if (-dot0d(tf, n3) > -f0 * 0.5f) {
    //         break;
    //     }
    // }
    // rb.set_tire_angular_velocity(tire_id, vv / rb.get_tire_radius(tire_id));
}

void idle(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& surface_normal,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    rb.set_tire_angular_velocity(tire_id, rb.get_angular_velocity_at_tire(surface_normal, street_velocity, tire_id));
    force_min = 0;
    force_max = 0;
}

FixedArray<float, 3> Mlib::updated_tire_speed(
    const PowerIntent& P,
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    // F = W / s = W / v / t = P / v
    if (!std::isnan(P.power)) {
        // std::cerr << "dx " << dx << std::endl;
        if (P.power != 0) {
            float v = dot0d(rb.rbi_.rbp_.v_ - street_velocity, n3);
            if (sign(P.power) != sign(v) && std::abs(v) > cfg.hand_brake_velocity) {
                if (P.power > 0) {
                    break_positive(rb, street_velocity, surface_normal, cfg, tire_id, force_min, force_max);
                } else if (P.power < 0) {
                    break_negative(rb, street_velocity, surface_normal, cfg, tire_id, force_min, force_max);
                }
            } else if (P.power > 0) {
                if (P.type == PowerIntentType::BREAK_OR_IDLE) {
                    idle(rb, street_velocity, surface_normal, tire_id, force_min, force_max);
                } else {
                    accelerate_positive(rb, street_velocity, P.power, vc, v0, surface_normal, cfg, tire_id, force_min, force_max);
                }
            } else if (P.power < 0) {
                if (P.type == PowerIntentType::BREAK_OR_IDLE) {
                    idle(rb, street_velocity, surface_normal, tire_id, force_min, force_max);
                } else {
                    accelerate_negative(rb, street_velocity, P.power, vc, v0, surface_normal, cfg, tire_id, force_min, force_max);
                }
            }
        } else {
            idle(rb, street_velocity, surface_normal, tire_id, force_min, force_max);
        }
    } else {
        float v = dot0d(rb.rbi_.rbp_.v_ - street_velocity, n3);
        if (v < 0) {
            break_positive(rb, street_velocity, surface_normal, cfg, tire_id, force_min, force_max);
        } else {
            break_negative(rb, street_velocity, surface_normal, cfg, tire_id, force_min, force_max);
        }
    }
    float v1 = rb.get_tire_angular_velocity(tire_id) * rb.get_tire_radius(tire_id);
    return n3 * v1;
}

FixedArray<float, 3> Mlib::handle_tire_triangle_intersection(
    RigidBody& rb,
    const FixedArray<float, 3>& street_velocity,
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    const FixedArray<float, 3>& surface_normal,
    float stiction_force,
    float friction_force,
    const PhysicsEngineConfig& cfg,
    size_t tire_id)
{
    float force_min;
    float force_max;
    FixedArray<float, 3> v_at_tire = rb.get_velocity_at_tire_contact(surface_normal, tire_id);
    return friction_force_infinite_mass(
        stiction_force,
        friction_force,
        v_at_tire + updated_tire_speed(
            rb.consume_tire_surface_power(tire_id),
            rb,
            street_velocity,
            vc,
            n3,
            -dot0d(v_at_tire, n3),
            surface_normal,
            cfg,
            tire_id,
            force_min,
            force_max),
        cfg.alpha0);
}
