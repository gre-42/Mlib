#include "Crash.hpp"
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Physics/Advance_Times/Damageable.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

Crash::Crash(float damage)
: damage_{damage}
{}

void Crash::notify_impact(
    const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
    float lambda_final)
{
    for(auto& v : collision_observers) {
        auto d = dynamic_cast<Damageable*>(v.get());
        if (d != nullptr) {
            d->damage(damage_ * squared(std::max(0.f, -lambda_final)));
        }
    }
}
