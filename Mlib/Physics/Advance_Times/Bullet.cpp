#include "Bullet.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

Bullet::Bullet(
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    SceneNode& bullet_node,
    AdvanceTimes& advance_times,
    RigidBody& rigid_body,
    const std::string& bullet_node_name,
    float max_lifetime,
    float damage,
    DeleteNodeMutex& delete_node_mutex)
: scene_{ scene },
  scene_node_resources_{ scene_node_resources },
  advance_times_{ advance_times },
  rigid_body_integrator_{ rigid_body.rbi_ },
  bullet_node_name_{ bullet_node_name },
  max_lifetime_{ max_lifetime },
  lifetime_{ 0 },
  damage_{ damage },
  delete_node_mutex_{ delete_node_mutex }
{
    bullet_node.add_destruction_observer(this);
}

void Bullet::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void Bullet::advance_time(float dt) {
    lifetime_ += dt;
    if (lifetime_ > max_lifetime_) {
        std::lock_guard lock{ delete_node_mutex_ };
        scene_.delete_root_node(bullet_node_name_);
    } else {
        rigid_body_integrator_.rbp_.rotation_ = gl_lookat_relative(rigid_body_integrator_.rbp_.v_ / std::sqrt(sum(squared(rigid_body_integrator_.rbp_.v_))));
    }
}

void Bullet::notify_collided(
    const FixedArray<float, 3>& intersection_point,
    RigidBody& rigid_body,
    CollisionRole collision_role,
    CollisionType& collision_type,
    bool& abort)
{
    if (lifetime_ == INFINITY) {
        abort = true;
        return;
    }
    lifetime_ = INFINITY;
    collision_type = CollisionType::GO_THROUGH;
    if (rigid_body.damageable_ != nullptr) {
        rigid_body.damageable_->damage(damage_);
    }

    auto node = std::make_unique<SceneNode>();
    node->set_position(intersection_point);
    node->set_style(new Style{.texture_animation = AnimationFrame{
        .begin = 0.f,
        .end = 0.3f,
        .time = 0.f}});
    scene_node_resources_.instantiate_renderable(
        "explosion_01",
        "explosion_01",
        *node,
        SceneNodeResourceFilter());
    std::string explosion_node_name = "explosion-" + std::to_string(scene_.get_uuid());
    scene_.add_root_node(explosion_node_name, std::move(node));
}
