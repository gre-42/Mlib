#include "Bullet.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

Bullet::Bullet(
    Scene& scene,
    SmokeParticleGenerator& smoke_generator,
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
  smoke_generator_{smoke_generator},
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
  trail_generator_{ smoke_generator },
  has_trail_{ !trail_resource.empty() },
  trail_resource_name_{ trail_resource },
  trail_animation_duration_{ trail_animation_time },
  trail_dt_{ trail_dt },
  delete_node_mutex_{ delete_node_mutex }
{}

Bullet::~Bullet() {
    advance_times_.delete_advance_time(*this);
    if (gunner_ != nullptr) {
        gunner_->notify_bullet_destroyed(*this);
    }
    if (team_ != nullptr) {
        team_->notify_bullet_destroyed(*this);
    }
}

void Bullet::notify_destroyed(const Object& destroyed_object) {
    if (dynamic_cast<const IPlayer*>(&destroyed_object) == gunner_) {
        gunner_ = nullptr;
    } else if (dynamic_cast<const ITeam*>(&destroyed_object) == team_) {
        team_ = nullptr;
    } else {
        THROW_OR_ABORT("Unexpected destruction notifier");
    }
}

void Bullet::advance_time(float dt) {
    lifetime_ += dt;
    if (lifetime_ > max_lifetime_) {
        std::scoped_lock lock{ delete_node_mutex_ };
        scene_.schedule_delete_root_node(bullet_node_name_);
        lifetime_ = INFINITY;
        return;
    }
    rigid_body_pulses_.rotation_ = gl_lookat_relative(rigid_body_pulses_.v_ / std::sqrt(sum(squared(rigid_body_pulses_.v_))));
    if (has_trail_) {
        trail_generator_.advance_time(dt);
        trail_generator_.maybe_generate(
            rigid_body_pulses_.abs_position(),
            trail_resource_name_,
            "trail",
            trail_animation_duration_,
            trail_dt_,
            ParticleType::INSTANCE);
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
    smoke_generator_.generate_root(
        bullet_explosion_resource_name_,
        "explosion" + smoke_generator_.generate_suffix(),
        intersection_point,
        bullet_explosion_animation_time_,
        ParticleType::NODE);
}

void Bullet::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    if (gunner_ != nullptr) {
        gunner_->notify_kill(rigid_body_vehicle);
    }
    if (team_ != nullptr) {
        team_->notify_kill(rigid_body_vehicle);
    }
}

void Bullet::cause_damage(RigidBodyVehicle& rigid_body, float amount) {
    if (rigid_body.damageable_ == nullptr) {
        return;
    }
    if (rigid_body.damageable_->health() <= 0.f) {
        return;
    }
    rigid_body.damageable_->damage(amount);
    if (rigid_body.damageable_->health() <= 0.f) {
        notify_kill(const_cast<RigidBodyVehicle&>(rigid_body));
        for (auto& p : rigid_body.passengers_) {
            cause_damage(*p, INFINITY);
        }
    }
}

void Bullet::cause_damage(
    const FixedArray<double, 3>& intersection_point,
    RigidBodyVehicle& rigid_body)
{
    if (damage_radius_ == 0) {
        cause_damage(rigid_body, damage_);
    } else {
        for (const auto& rbm : rigid_bodies_.objects()) {
            const RigidBodyVehicle& rb = rbm.rigid_body;
            if ((rb.damageable_ == nullptr) ||
                (rb.damageable_->health() <= 0.f))
            {
                continue;
            }
            double dist2 = sum(squared(rb.rbi_.abs_position() - intersection_point));
            if (dist2 > squared(damage_radius_)) {
                continue;
            }
            cause_damage(const_cast<RigidBodyVehicle&>(rb), damage_);
        }
    }
}
