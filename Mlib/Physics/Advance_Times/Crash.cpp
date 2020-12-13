#include "Crash.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Advance_Times/Damageable.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Pulses.hpp>

using namespace Mlib;

Crash::Crash(float damage)
: damage_{damage}
{}

void Crash::notify_impact(
    const RigidBodyPulses& rbp,
    const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
    const FixedArray<float, 3>& normal,
    float lambda_final)
{
    for(auto& v : collision_observers) {
        auto d = dynamic_cast<Damageable*>(v.get());
        if (d != nullptr) {
            float fac = dot0d(rbp.abs_z(), normal);
            fac = std::sqrt(1 - squared(fac));
            d->damage(damage_ * squared(std::max(0.f, -lambda_final)) * fac);
        }
    }
}
