#include "Bullet.hpp"
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Physics/Advance_Times/Damageable.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

Bullet::Bullet(
    Scene& scene,
    SceneNode& bullet_node,
    AdvanceTimes& advance_times,
    RigidBody& rigid_body,
    const std::string& bullet_node_name,
    float max_lifetime,
    float damage)
: scene_{scene},
  advance_times_{advance_times},
  rigid_body_integrator_{rigid_body.rbi_},
  bullet_node_name_{bullet_node_name},
  max_lifetime_{max_lifetime},
  lifetime_{0},
  damage_{damage}
{
    bullet_node.add_destruction_observer(this);
}

void Bullet::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void Bullet::advance_time(float dt) {
    lifetime_ += dt;
    if (lifetime_ > max_lifetime_) {
        scene_.delete_root_node(bullet_node_name_);
    } else {
        rigid_body_integrator_.rotation_ = lookat(rigid_body_integrator_.v_ / std::sqrt(sum(squared(rigid_body_integrator_.v_))));
    }
}

void Bullet::notify_collided(
    const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
    CollisionType& collision_type,
    bool& abort)
{
    if (lifetime_ == INFINITY) {
        abort = true;
        return;
    }
    lifetime_ = INFINITY;
    collision_type = CollisionType::GO_THROUGH;
    for(auto& v : collision_observers) {
        auto d = dynamic_cast<Damageable*>(v.get());
        if (d != nullptr) {
            d->damage(damage_);
        }
    }
}
