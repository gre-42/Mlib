#include "Handle_Tire_Triangle_Intersection.hpp"
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>

using namespace Mlib;

void optimal_angular_velocity_positive(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& w,
    float* v = nullptr)
{
    float vv = rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id) * rb.get_tire_radius(tire_id);
    float y = std::max(cfg.hand_brake_velocity, std::abs(vv));
    float m = rb.tires_.at(tire_id).magic_formula.longitudinal().argmax;
    w = (m * y - vv) / rb.get_tire_radius(tire_id);
    if (v != nullptr) {
        *v = vv;
    }
}

void optimal_angular_velocity_negative(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& w,
    float* v = nullptr)
{
    float vv = rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id) * rb.get_tire_radius(tire_id);
    float y = std::max(cfg.hand_brake_velocity, std::abs(vv));
    float m = -rb.tires_.at(tire_id).magic_formula.longitudinal().argmax;
    w = (m * y - vv) / rb.get_tire_radius(tire_id);
    if (v != nullptr) {
        *v = vv;
    }
}

void accelerate_positive(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float power,
    const FixedArray<float, 3>& vc,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    // r = v / w => more power needed on outer wheel, because p = f * v and dv = f / m * dt
    float u = 1;
    if (-v0 > 1e-12) {
        if (float vc2 = sum(squared(vc)); vc2 > 1e-12) {
            u = std::clamp<float>(-v0 / std::sqrt(vc2), float(1e-1), float(1e1));
        }
    }
    float w;
    float v;
    optimal_angular_velocity_positive(rb, v_street, surface_normal, cfg, tire_id, w, &v);
    rb.set_tire_angular_velocity(tire_id, -w, TireAngularVelocityChange::ACCELERATE);
    force_min = u * power / std::min(-0.001f, v);
    force_max = 0;
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        *(int*)nullptr = 42;
    }
}

void accelerate_negative(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float power,
    const FixedArray<float, 3>& vc,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    // r = v / w => more power needed on outer wheel, because p = f * v and dv = f / m * dt
    float u = 1;
    if (v0 > 1e-12) {
        if (float vc2 = sum(squared(vc)); vc2 > 1e-12) {
            u = std::clamp<float>(v0 / std::sqrt(vc2), float(1e-1), float(1e1));
        }
    }
    float w;
    float v;
    optimal_angular_velocity_negative(rb, v_street, surface_normal, cfg, tire_id, w, &v);
    rb.set_tire_angular_velocity(tire_id, -w, TireAngularVelocityChange::ACCELERATE);
    force_min = 0;
    force_max = -u * power / std::max(0.001f, v);
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        *(int*)nullptr = 42;
    }
}

void break_positive(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float w;
    optimal_angular_velocity_positive(rb, v_street, surface_normal, cfg, tire_id, w);
    if (sign(rb.get_tire_angular_velocity(tire_id)) != sign(-w)) {
        rb.set_tire_angular_velocity(tire_id, 0, TireAngularVelocityChange::BREAK);
    } else {
        rb.set_tire_angular_velocity(tire_id, -w, TireAngularVelocityChange::BREAK);
    }
    force_min = -rb.tires_.at(tire_id).break_force;
    force_max = 0;
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        *(int*)nullptr = 42;
    }
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
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float w;
    optimal_angular_velocity_negative(rb, v_street, surface_normal, cfg, tire_id, w);
    if (sign(rb.get_tire_angular_velocity(tire_id)) != sign(-w)) {
        rb.set_tire_angular_velocity(tire_id, 0, TireAngularVelocityChange::BREAK);
    } else {
        rb.set_tire_angular_velocity(tire_id, -w, TireAngularVelocityChange::BREAK);
    }
    force_min = 0;
    force_max = rb.tires_.at(tire_id).break_force;
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        *(int*)nullptr = 42;
    }
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
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& surface_normal,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    rb.set_tire_angular_velocity(tire_id, rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id), TireAngularVelocityChange::IDLE);
    force_min = 0;
    force_max = 0;
    if (force_min > force_max) {
        *(int*)nullptr = 42;
    }
}

void Mlib::handle_tire_triangle_intersection(
    const PowerIntent& P,
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& vc_street,
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    bool c;
    switch (rb.vehicle_controller().steering_type) {
        case SteeringType::CAR:
            c = true;
            break;
        case SteeringType::TANK:
            c = false;
            break;
        default:
            throw std::runtime_error("Unknown steering type");
    }
    // F = W / s = W / v / t = P / v
    if (!std::isnan(P.power)) {
        // std::cerr << "dx " << dx << std::endl;
        if (P.power != 0) {
            float v = c ? dot0d(vc - vc_street, n3) : -v0;
            if (sign(P.power) != sign(v) && std::abs(v) > cfg.hand_brake_velocity) {
                if (v0 > 0) {
                    break_positive(rb, v_street, surface_normal, cfg, tire_id, force_min, force_max);
                } else if (v0 < 0) {
                    break_negative(rb, v_street, surface_normal, cfg, tire_id, force_min, force_max);
                } else {
                    idle(rb, v_street, surface_normal, tire_id, force_min, force_max);
                }
            } else if (P.power > 0) {
                if (P.type == PowerIntentType::BREAK_OR_IDLE) {
                    idle(rb, v_street, surface_normal, tire_id, force_min, force_max);
                } else {
                    accelerate_positive(rb, v_street, P.power, c ? vc : fixed_zeros<float, 3>(), c ? v0 : 0.f, surface_normal, cfg, tire_id, force_min, force_max);
                }
            } else if (P.power < 0) {
                if (P.type == PowerIntentType::BREAK_OR_IDLE) {
                    idle(rb, v_street, surface_normal, tire_id, force_min, force_max);
                } else {
                    accelerate_negative(rb, v_street, P.power, c ? vc : fixed_zeros<float, 3>(), c ? v0 : 0.f, surface_normal, cfg, tire_id, force_min, force_max);
                }
            } else {
                throw std::runtime_error("handle_tire_triangle_intersection internal error");
            }
        } else {
            idle(rb, v_street, surface_normal, tire_id, force_min, force_max);
        }
    } else {
        if (v0 > 0) {
            break_positive(rb, v_street, surface_normal, cfg, tire_id, force_min, force_max);
        } else if (v0 < 0) {
            break_negative(rb, v_street, surface_normal, cfg, tire_id, force_min, force_max);
        } else {
            idle(rb, v_street, surface_normal, tire_id, force_min, force_max);
        }
    }
    if (force_min > force_max) {
        *(int*)nullptr = 42;
    }
}
