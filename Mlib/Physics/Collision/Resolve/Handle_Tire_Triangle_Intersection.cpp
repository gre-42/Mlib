#include "Handle_Tire_Triangle_Intersection.hpp"
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Actuators/Tire_Power_Intent.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static void optimal_angular_velocity_positive(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float relaxation,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& w,
    float* v = nullptr)
{
    float vv = rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id) * rb.get_tire_radius(tire_id);
    float y = std::max(cfg.hand_brake_velocity, std::abs(vv));
    float m = rb.tires_.get(tire_id).magic_formula.longitudinal().argmax;
    w = (relaxation * m * y - vv) / rb.get_tire_radius(tire_id);
    if (v != nullptr) {
        *v = vv;
    }
}

static void optimal_angular_velocity_negative(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float relaxation,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    size_t tire_id,
    float& w,
    float* v = nullptr)
{
    float vv = rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id) * rb.get_tire_radius(tire_id);
    float y = std::max(cfg.hand_brake_velocity, std::abs(vv));
    float m = -rb.tires_.get(tire_id).magic_formula.longitudinal().argmax;
    w = (relaxation * m * y - vv) / rb.get_tire_radius(tire_id);
    if (v != nullptr) {
        *v = vv;
    }
}

static void accelerate_positive(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float power,
    float relaxation,
    const FixedArray<float, 3>& vc,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
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
    optimal_angular_velocity_positive(rb, v_street, relaxation, surface_normal, cfg, tire_id, w, &v);
    rb.set_tire_angular_velocity(tire_id, -w, cfg, phase, power);
    force_min = u * power / std::min(-0.001f, v);
    force_max = 0;
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        THROW_OR_ABORT("accelerate_positive: forces out of bounds");
    }
}

static void accelerate_negative(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float power,
    float relaxation,
    const FixedArray<float, 3>& vc,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
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
    optimal_angular_velocity_negative(rb, v_street, relaxation, surface_normal, cfg, tire_id, w, &v);
    rb.set_tire_angular_velocity(tire_id, -w, cfg, phase, power);
    force_min = 0;
    force_max = -u * power / std::max(0.001f, v);
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        THROW_OR_ABORT("accelerate_negative: forces out of bounds");
    }
}

void brake_positive(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float relaxation,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float w;
    optimal_angular_velocity_positive(rb, v_street, relaxation, surface_normal, cfg, tire_id, w);
    if (sign(rb.get_tire_angular_velocity(tire_id)) != sign(-w)) {
        float available_power = 0.f;
        rb.set_tire_angular_velocity(tire_id, 0, cfg, phase, available_power);
    } else {
        float available_power = 0.f;
        rb.set_tire_angular_velocity(tire_id, -w, cfg, phase, available_power);
    }
    force_min = -rb.tires_.get(tire_id).brake_force;
    force_max = 0;
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        THROW_OR_ABORT("brake_positive: forces out of bounds");
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

void brake_negative(
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    float relaxation,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float w;
    optimal_angular_velocity_negative(rb, v_street, relaxation, surface_normal, cfg, tire_id, w);
    if (sign(rb.get_tire_angular_velocity(tire_id)) != sign(-w)) {
        float available_power = 0.f;
        rb.set_tire_angular_velocity(tire_id, 0, cfg, phase, available_power);
    } else {
        float available_power = 0.f;
        rb.set_tire_angular_velocity(tire_id, -w, cfg, phase, available_power);
    }
    force_min = 0;
    force_max = rb.tires_.get(tire_id).brake_force;
    if ((force_min > force_max) || (std::abs(force_min) > 1e9) || (std::abs(force_max) > 1e9)) {
        THROW_OR_ABORT("brake_negative: forces out of bounds");
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
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    float available_power = 0.f;
    rb.set_tire_angular_velocity(tire_id, rb.get_angular_velocity_at_tire(surface_normal, v_street, tire_id), cfg, phase, available_power);
    force_min = 0;
    force_max = 0;
}

void Mlib::handle_tire_triangle_intersection(
    const TirePowerIntent& P,
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& vc_street,
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    size_t tire_id,
    float& force_min,
    float& force_max)
{
    rb.verify_tire_angular_velocity(tire_id);

    bool c;
    switch (rb.vehicle_controller().steering_type) {
        case SteeringType::CAR:
            c = true;
            break;
        case SteeringType::TANK:
            c = false;
            break;
        default:
            THROW_OR_ABORT("Unknown steering type");
    }
    // F = W / s = W / v / t = P / v
    if (!std::isnan(P.power)) {
        // lerr() << "dx " << dx;
        if (P.power != 0) {
            if (P.type == TirePowerIntentType::BRAKE_OR_IDLE) {
                if ((v0 > 0) && (P.power > 0)) {
                    brake_positive(rb, v_street, P.relaxation, surface_normal, cfg, phase, tire_id, force_min, force_max);
                } else if ((v0 < 0) && (P.power < 0)) {
                    brake_negative(rb, v_street, P.relaxation, surface_normal, cfg, phase, tire_id, force_min, force_max);
                } else {
                    idle(rb, v_street, surface_normal, cfg, phase, tire_id, force_min, force_max);
                }
            } else if (P.power > 0) {
                accelerate_positive(rb, v_street, P.power, P.relaxation, c ? vc : fixed_zeros<float, 3>(), c ? v0 : 0.f, surface_normal, cfg, phase, tire_id, force_min, force_max);
            } else if (P.power < 0) {
                accelerate_negative(rb, v_street, P.power, P.relaxation, c ? vc : fixed_zeros<float, 3>(), c ? v0 : 0.f, surface_normal, cfg, phase, tire_id, force_min, force_max);
            } else {
                THROW_OR_ABORT("handle_tire_triangle_intersection internal error");
            }
        } else {
            idle(rb, v_street, surface_normal, cfg, phase, tire_id, force_min, force_max);
        }
    } else {
        if (v0 > 0) {
            brake_positive(rb, v_street, P.relaxation, surface_normal, cfg, phase, tire_id, force_min, force_max);
        } else if (v0 < 0) {
            brake_negative(rb, v_street, P.relaxation, surface_normal, cfg, phase, tire_id, force_min, force_max);
        } else {
            idle(rb, v_street, surface_normal, cfg, phase, tire_id, force_min, force_max);
        }
    }
    if (force_min > force_max) {
        THROW_OR_ABORT("handle_tire_triangle_intersection: force_min > force_max");
    }
    rb.verify_tire_angular_velocity(tire_id);
}
