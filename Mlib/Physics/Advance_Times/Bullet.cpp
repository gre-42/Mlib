#include "Bullet.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

Bullet::Bullet(
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    AdvanceTimes& advance_times,
    RigidBodyVehicle& rigid_body,
    RigidBodies& rigid_bodies,
    IPlayer* gunner,
    ITeam* team,
    const std::string& bullet_node_name,
    const std::string& bullet_explosion_resource_name,
    float bullet_explosion_animation_time,
    float max_lifetime,
    float damage,
    float damage_radius,
    const std::string& trail_resource,
    float trail_dt,
    float trail_animation_time,
    DeleteNodeMutex& delete_node_mutex)
: scene_{ scene },
  scene_node_resources_{ scene_node_resources },
  advance_times_{ advance_times },
  rigid_body_pulses_{ rigid_body.rbi_.rbp_ },
  rigid_bodies_{ rigid_bodies },
  gunner_{ gunner },
  team_{ team },
  bullet_node_name_{ bullet_node_name },
  bullet_explosion_resource_name_{ bullet_explosion_resource_name },
  bullet_explosion_animation_time_{ bullet_explosion_animation_time },
  max_lifetime_{ max_lifetime },
  lifetime_{ 0.f },
  damage_{ damage },
  damage_radius_{ damage_radius },
  trail_resource_{ trail_resource },
  trail_dt_{ trail_dt },
  trail_animation_time_{ trail_animation_time },
  trail_lifetime_{ 0.f },
  delete_node_mutex_{ delete_node_mutex }
{}

Bullet::~Bullet() {
    if (gunner_ != nullptr) {
        gunner_->notify_bullet_destroyed(this);
    }
    if (team_ != nullptr) {
        team_->notify_bullet_destroyed(this);
    }
}

void Bullet::notify_destroyed(Object* obj) {
    if (dynamic_cast<IPlayer*>(obj) == gunner_) {
        gunner_ = nullptr;
    } else if (dynamic_cast<ITeam*>(obj) == team_) {
        team_ = nullptr;
    } else {
        advance_times_.schedule_delete_advance_time(this);
    }
}

void Bullet::advance_time(float dt) {
    lifetime_ += dt;
    if (lifetime_ > max_lifetime_) {
        std::lock_guard lock{ delete_node_mutex_ };
        scene_.delete_root_node(bullet_node_name_);
        return;
    }
    rigid_body_pulses_.rotation_ = gl_lookat_relative(rigid_body_pulses_.v_ / std::sqrt(sum(squared(rigid_body_pulses_.v_))));
    if (!trail_resource_.empty()) {
        trail_lifetime_ += dt;
        if (trail_lifetime_ > trail_dt_) {
            trail_lifetime_ = 0.f;
            generate_trail();
        }
    }
}

void Bullet::notify_collided(
    const FixedArray<double, 3>& intersection_point,
    RigidBodyVehicle& rigid_body,
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
    cause_damage(intersection_point, rigid_body);
    generate_explosion(intersection_point);
}

void Bullet::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    if (gunner_ != nullptr) {
        gunner_->notify_kill(rigid_body_vehicle);
    }
    if (team_ != nullptr) {
        team_->notify_kill(rigid_body_vehicle);
    }
}

void Bullet::cause_damage(
    const FixedArray<double, 3>& intersection_point,
    RigidBodyVehicle& rigid_body)
{
    if (damage_radius_ == 0) {
        if ((rigid_body.damageable_ != nullptr) &&
            (rigid_body.damageable_->health() > 0.f))
        {
            rigid_body.damageable_->damage(damage_);
            if (rigid_body.damageable_->health() <= 0.f) {
                notify_kill(rigid_body);
            }
        }
    } else {
        rigid_bodies_.visit_rigid_bodies(
            [this, intersection_point](const RigidBodyVehicle& rb)
            {
                if ((rb.damageable_ == nullptr) ||
                    (rb.damageable_->health() <= 0.f))
                {
                    return;
                }
                double dist2 = sum(squared(rb.rbi_.abs_position() - intersection_point));
                if (dist2 > squared(damage_radius_)) {
                    return;
                }
                rb.damageable_->damage(damage_);
                if (rb.damageable_->health() <= 0.f) {
                    notify_kill(const_cast<RigidBodyVehicle&>(rb));
                }
            });
    }
}

void Bullet::generate_explosion(const FixedArray<double, 3>& intersection_point) {
    auto node = std::make_unique<SceneNode>();
    node->set_position(intersection_point);
    node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .aperiodic_animation_frame = AperiodicAnimationFrame{
            .frame = AnimationFrame{
                .begin = 0.f,
                .end = bullet_explosion_animation_time_,
                .time = 0.f}},
        .delete_node_when_aperiodic_animation_finished = true}));
    scene_node_resources_.instantiate_renderable(
        bullet_explosion_resource_name_,
        InstantiationOptions{
            .instance_name = "explosion",
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter{}});
    std::string explosion_node_name = "explosion-" + std::to_string(scene_.get_uuid());
    scene_.add_root_node(explosion_node_name, std::move(node));
}

void Bullet::generate_trail() {
    auto node = std::make_unique<SceneNode>();
    node->set_position(rigid_body_pulses_.abs_position());
    node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .aperiodic_animation_frame = AperiodicAnimationFrame{
            .frame = AnimationFrame{
                .begin = 0.f,
                .end = trail_animation_time_,
                .time = 0.f}},
        .delete_node_when_aperiodic_animation_finished = true}));
    scene_node_resources_.instantiate_renderable(
        trail_resource_,
        InstantiationOptions{
            .instance_name = "trail",
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter{}});
    std::string suffix = std::to_string(scene_.get_uuid());
    scene_.add_root_node("trail_node-" + suffix, std::move(node));
}
