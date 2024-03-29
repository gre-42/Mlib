#include "Bullet.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Lights.hpp>
#include <Mlib/Physics/Interfaces/IDamageable.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Light.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
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
    std::string bullet_node_name,
    const BulletProperties& props,
    std::unique_ptr<ITrailExtender> trace_extender,
    DynamicLights& dynamic_lights,
    DeleteNodeMutex& delete_node_mutex,
    std::chrono::steady_clock::time_point time)
    : scene_{ scene }
    , smoke_generator_{ smoke_generator }
    , advance_times_{ advance_times }
    , rigid_body_pulses_{ rigid_body.rbp_ }
    , rigid_bodies_{ rigid_bodies }
    , gunner_{ gunner }
    , team_{ team }
    , bullet_node_name_{ std::move(bullet_node_name) }
    , props_{ props }
    , lifetime_{ 0.f }
    , trail_generator_{ smoke_generator }
    , has_trail_{ !props.trail_resource_name.empty() }
    , trace_extender_{ std::move(trace_extender) }
    , dynamic_lights_{ dynamic_lights }
    , delete_node_mutex_{ delete_node_mutex }
{
    if (!props_.dynamic_light_configuration_before_impact.empty()) {
        auto func = [&b = rigid_body.rbp_]() { return b.abs_position(); };
        light_before_impact_ = dynamic_lights_.instantiate(props_.dynamic_light_configuration_before_impact, func, time);
    }
}

Bullet::~Bullet() {
    advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
    if (gunner_ != nullptr) {
        gunner_->notify_bullet_destroyed(*this);
    }
    if (team_ != nullptr) {
        team_->notify_bullet_destroyed(*this);
    }
}

void Bullet::notify_destroyed(const IPlayer& destroyed_object) {
    if (&destroyed_object == gunner_) {
        gunner_ = nullptr;
    } else {
        THROW_OR_ABORT("Unexpected destruction notifier");
    }
}

void Bullet::notify_destroyed(const ITeam& destroyed_object) {
    if (&destroyed_object == team_) {
        team_ = nullptr;
    } else {
        THROW_OR_ABORT("Unexpected destruction notifier");
    }
}

void Bullet::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    lifetime_ += dt;
    if (lifetime_ > props_.max_lifetime) {
        std::scoped_lock lock{ delete_node_mutex_ };
        scene_.schedule_delete_root_node(bullet_node_name_);
        lifetime_ = INFINITY;
        return;
    }
    auto R = gl_lookat_relative(
        rigid_body_pulses_.v_ / std::sqrt(sum(squared(rigid_body_pulses_.v_))),
        rigid_body_pulses_.rotation_.column(1));
    if (!R.has_value()) {
        THROW_OR_ABORT("Could not update bullet rotation");
    }
    rigid_body_pulses_.rotation_ = R.value();
    if (has_trail_) {
        trail_generator_.advance_time(dt);
        trail_generator_.maybe_generate(
            rigid_body_pulses_.abs_position(),
            fixed_zeros<float, 3>(),
            props_.trail_resource_name,
            "trail",
            props_.trail_animation_duration,
            props_.trail_dt,
            ParticleType::INSTANCE);
    }
    if (trace_extender_ != nullptr) {
        trace_extender_->append_location(rigid_body_pulses_.abs_transformation());
    }
}

void Bullet::notify_collided(
    const FixedArray<double, 3>& intersection_point,
    std::chrono::steady_clock::time_point time,
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
    if (!props_.dynamic_light_configuration_after_impact.empty()) {
        auto func = [&b = rigid_body.rbp_]() { return b.abs_position(); };
        light_after_impact_ = dynamic_lights_.instantiate(props_.dynamic_light_configuration_after_impact, func, time);
    }
    if (light_before_impact_ != nullptr) {
        light_before_impact_ = nullptr;
    }
    smoke_generator_.generate_root(
        props_.explosion_resource_name,
        "explosion" + smoke_generator_.generate_suffix(),
        intersection_point,
        fixed_zeros<float, 3>(),
        props_.explosion_animation_time,
        ParticleType::NODE);
    if (trace_extender_ != nullptr) {
        trace_extender_->append_location(TransformationMatrix<float, double, 3>{rigid_body_pulses_.rotation_, intersection_point});
    }
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
    if (props_.damage_radius == 0) {
        cause_damage(rigid_body, props_.damage);
    } else {
        for (const auto& rbm : rigid_bodies_.objects()) {
            const RigidBodyVehicle& rb = rbm.rigid_body;
            if ((rb.damageable_ == nullptr) ||
                (rb.damageable_->health() <= 0.f))
            {
                continue;
            }
            double dist2 = sum(squared(rb.rbp_.abs_position() - intersection_point));
            if (dist2 > squared(props_.damage_radius)) {
                continue;
            }
            cause_damage(const_cast<RigidBodyVehicle&>(rb), props_.damage);
        }
    }
}
