#include "Crash.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Advance_Times/Damageable.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Pulses.hpp>
#include <Mlib/Scene_Graph/Base_Log.hpp>
#include <Mlib/Scene_Graph/Log_Entry_Severity.hpp>

using namespace Mlib;

Crash::Crash(
    const RigidBodyPulses& rbp,
    const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
    float damage)
: rbp_{rbp},
  collision_observers_{collision_observers},
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
    const RigidBodyPulses& rbp,
    const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
    CollisionRole collision_role,
    const FixedArray<float, 3>& normal,
    float lambda_final,
    BaseLog* base_log)
{
    if (collision_role == CollisionRole::PRIMARY) {
        float damage0 = NAN;
        for(auto& v : collision_observers) {
            auto d = dynamic_cast<Crash*>(v.get());
            if (d != nullptr) {
                if (!std::isnan(damage0)) {
                    throw std::runtime_error("List contains multiple crashes");
                }
                damage0 = calculate_damage(rbp_, d->damage_, normal, lambda_final);
            }
        }
        if (!std::isnan(damage0)) {
            float damage1 = calculate_damage(rbp, damage_, normal, lambda_final);
            float min_damage = std::min(damage0, damage1);
            damage0 -= min_damage;
            damage1 -= min_damage;
            if (base_log != nullptr) {
                std::stringstream sstr;
                sstr << "Damage: " << damage0 << " " << damage1;
                if (damage0 < 1 && damage1 < 1) {
                    base_log->log(sstr.str(), LogEntrySeverity::INFO);
                } else {
                    base_log->log(sstr.str(), LogEntrySeverity::CRITICAL);
                }
            }
            for(auto& v : collision_observers) {
                auto d = dynamic_cast<Damageable*>(v.get());
                if (d != nullptr) {
                    d->damage(damage1);
                }
            }
            for(auto& v : collision_observers_) {
                auto d = dynamic_cast<Damageable*>(v.get());
                if (d != nullptr) {
                    d->damage(damage0);
                }
            }
        }
    }
}
