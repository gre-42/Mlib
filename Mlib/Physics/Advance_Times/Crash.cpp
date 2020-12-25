#include "Crash.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Scene_Graph/Base_Log.hpp>
#include <Mlib/Scene_Graph/Log_Entry_Severity.hpp>

using namespace Mlib;

Crash::Crash(
    RigidBody& rigid_body,
    float damage)
: rigid_body_{rigid_body},
  damage_{damage}
{}

float calculate_damage(
    const RigidBodyPulses& rbp,
    float damage_raw,
    const FixedArray<float, 3>& normal,
    float lambda_final)
{
    float fac = dot0d(rbp.abs_z(), normal);
    fac = std::sqrt(1 - std::min(1.f, squared(fac)));
    return damage_raw * squared(std::max(0.f, -lambda_final)) * fac;
}

void Crash::notify_impact(
    RigidBody& rigid_body,
    CollisionRole collision_role,
    const FixedArray<float, 3>& normal,
    float lambda_final,
    BaseLog* base_log)
{
    if (collision_role == CollisionRole::PRIMARY) {
        float damage0 = NAN;
        for (auto& v : rigid_body.collision_observers_) {
            auto d = dynamic_cast<Crash*>(v.get());
            if (d != nullptr) {
                if (!std::isnan(damage0)) {
                    throw std::runtime_error("List contains multiple crashes");
                }
                damage0 = calculate_damage(rigid_body_.rbi_.rbp_, d->damage_, normal, lambda_final);
            }
        }
        if (!std::isnan(damage0)) {
            float damage1 = calculate_damage(rigid_body.rbi_.rbp_, damage_, normal, lambda_final);
            float min_damage = std::min(damage0, damage1);
            damage0 -= min_damage;
            damage1 -= min_damage;
            if (base_log != nullptr) {
                std::stringstream sstr;
                if ((rigid_body_.driver_ != nullptr) && (rigid_body.driver_ != nullptr)) {
                    if (damage0 != 0) {
                        sstr << rigid_body.driver_->name() << " -> " << rigid_body_.driver_->name() << ": " << std::round(damage0) << " HP";
                    }
                    if (damage1 != 0) {
                        sstr << rigid_body_.driver_->name() << " -> " << rigid_body.driver_->name() << ": " << std::round(damage1) << " HP";
                    }
                }
                if (damage0 < 1 && damage1 < 1) {
                    base_log->log(sstr.str(), LogEntrySeverity::INFO);
                } else {
                    base_log->log(sstr.str(), LogEntrySeverity::CRITICAL);
                }
            }
            if (rigid_body_.damageable_ != nullptr) {
                rigid_body_.damageable_->damage(damage0);
            }
            if (rigid_body.damageable_ != nullptr) {
                rigid_body.damageable_->damage(damage1);
            }
        }
    }
}
