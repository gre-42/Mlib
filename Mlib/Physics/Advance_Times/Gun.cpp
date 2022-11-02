#include "Gun.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Team/Team.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

Gun::Gun(
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    RigidBodies& rigid_bodies,
    AdvanceTimes& advance_times,
    float cool_down,
    RigidBodyVehicle& parent_rb,
    SceneNode& node,
    SceneNode& punch_angle_node,
    const std::string& bullet_renderable_resource_name,
    const std::string& bullet_hitbox_resource_name,
    const std::string& bullet_explosion_resource_name,
    float bullet_explosion_animation_time,
    bool bullet_feels_gravity,
    float bullet_mass,
    float bullet_velocity,
    float bullet_lifetime,
    float bullet_damage,
    float bullet_damage_radius,
    const FixedArray<float, 3>& bullet_size,
    const std::string& bullet_trail_resource,
    float bullet_trail_dt,
    float bullet_trail_animation_time,
    const std::string& ammo_type,
    const std::function<FixedArray<float, 3>(bool shooting)>& punch_angle_rng,
    const std::string& muzzle_flash_resource,
    const FixedArray<float, 3>& muzzle_flash_position,
    float muzzle_flash_animation_time,
    const std::function<void(const std::string& muzzle_flash_suffix)>& generate_muzzle_flash_hider,
    DeleteNodeMutex& delete_node_mutex)
: scene_{ scene },
  scene_node_resources_{ scene_node_resources },
  rigid_bodies_{ rigid_bodies },
  advance_times_{ advance_times },
  parent_rb_{ parent_rb },
  node_{ node },
  punch_angle_node_{ punch_angle_node },
  bullet_renderable_resource_name_{ bullet_renderable_resource_name },
  bullet_hitbox_resource_name_{ bullet_hitbox_resource_name },
  bullet_explosion_resource_name_{ bullet_explosion_resource_name },
  bullet_explosion_animation_time_{ bullet_explosion_animation_time },
  bullet_feels_gravity_{ bullet_feels_gravity },
  bullet_mass_{ bullet_mass },
  bullet_velocity_{ bullet_velocity },
  bullet_lifetime_{ bullet_lifetime },
  bullet_damage_{ bullet_damage },
  bullet_damage_radius_{ bullet_damage_radius },
  bullet_size_{ bullet_size },
  bullet_trail_resource_{ bullet_trail_resource },
  bullet_trail_dt_{ bullet_trail_dt },
  bullet_trail_animation_time_{ bullet_trail_animation_time},
  ammo_type_{ ammo_type },
  triggered_{ false },
  cool_down_{ cool_down },
  time_since_last_shot_{ 0 },
  absolute_model_matrix_{ fixed_nans<double, 4, 4 >() },
  punch_angle_{ 0.f, 0.f, 0.f },
  punch_angle_rng_{ punch_angle_rng },
  muzzle_flash_resource_{ muzzle_flash_resource },
  muzzle_flash_position_{ muzzle_flash_position },
  muzzle_flash_animation_time_{ muzzle_flash_animation_time },
  generate_muzzle_flash_hider_{ generate_muzzle_flash_hider },
  delete_node_mutex_{ delete_node_mutex }
{}

void Gun::advance_time(float dt) {
    time_since_last_shot_ += dt;
    time_since_last_shot_ = std::min(time_since_last_shot_, cool_down_);
    punch_angle_ = punch_angle_rng_(maybe_generate_bullet());
    punch_angle_node_.set_rotation(punch_angle_);
    triggered_ = false;
}

size_t Gun::nbullets_available() const {
    return parent_rb_.inventory_.navailable(ammo_type_);
}

bool Gun::maybe_generate_bullet() {
    if (is_none_gun()) {
        return false;
    }
    if (!triggered_) {
        return false;
    }
    if (time_since_last_shot_ != cool_down_) {
        return false;
    }
    if (nbullets_available() == 0) {
        return false;
    }
    parent_rb_.inventory_.take(ammo_type_, 1);
    time_since_last_shot_ = 0;
    generate_bullet();
    if (!muzzle_flash_resource_.empty()) {
        generate_muzzle_flash_hider();
    }
    return true;
}

void Gun::generate_bullet() {
    std::shared_ptr<RigidBodyVehicle> rc = rigid_cuboid("bullet", bullet_mass_, bullet_size_);
    rc->feels_gravity_ = bullet_feels_gravity_;
    auto node = std::make_unique<SceneNode>();
    FixedArray<double, 3> t = absolute_model_matrix_.t();
    FixedArray<float, 3> r = matrix_2_tait_bryan_angles(absolute_model_matrix_.R());
    node->set_position(t);
    node->set_rotation(r);
    node->set_absolute_movable(rc.get());
    rc->rbi_.rbp_.v_ =
        - bullet_velocity_ * z3_from_3x3(absolute_model_matrix_.R())
        + parent_rb_.rbi_.rbp_.v_;
    if (!bullet_renderable_resource_name_.empty()) {
        scene_node_resources_.instantiate_renderable(
            bullet_renderable_resource_name_,
            InstantiationOptions{
                .instance_name = "bullet",
                .scene_node = *node,
                .renderable_resource_filter = RenderableResourceFilter{}});
    }
    rigid_bodies_.add_rigid_body(
        rc,
        scene_node_resources_.get_animated_arrays(bullet_hitbox_resource_name_)->scvas,
        scene_node_resources_.get_animated_arrays(bullet_hitbox_resource_name_)->dcvas,
        CollidableMode::SMALL_MOVING,
        PhysicsResourceFilter{});
    std::string bullet_node_name = "bullet-" + std::to_string(scene_.get_uuid());
    auto bullet = std::make_shared<Bullet>(
        scene_,
        scene_node_resources_,
        advance_times_,
        *rc,
        rigid_bodies_,
        player_,
        team_,
        bullet_node_name,
        bullet_explosion_resource_name_,
        bullet_explosion_animation_time_,
        bullet_lifetime_,
        bullet_damage_,
        bullet_damage_radius_,
        bullet_trail_resource_,
        bullet_trail_dt_,
        bullet_trail_animation_time_,
        delete_node_mutex_);
    if (player_ != nullptr) {
        player_->destruction_observers.add(bullet.get());
    }
    if (team_ != nullptr) {
        team_->destruction_observers.add(bullet.get());
    }
    node->destruction_observers.add(bullet.get());
    rc->collision_observers_.push_back(bullet);
    advance_times_.add_advance_time(bullet);
    scene_.add_root_node(bullet_node_name, std::move(node));
}

void Gun::generate_muzzle_flash_hider() {
    auto muzzle_flash_node = std::make_unique<SceneNode>();
    muzzle_flash_node->set_position(muzzle_flash_position_.casted<double>());

    muzzle_flash_node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .aperiodic_animation_frame = AperiodicAnimationFrame{
            .frame = AnimationFrame{
                .begin = 0.f,
                .end = muzzle_flash_animation_time_,
                .time = 0.f}},
        .delete_node_when_aperiodic_animation_finished = true}));
    scene_node_resources_.instantiate_renderable(
        muzzle_flash_resource_,
        InstantiationOptions{
            .instance_name = "muzzle_flash",
            .scene_node = *muzzle_flash_node,
            .renderable_resource_filter = RenderableResourceFilter{}});
    std::string muzzle_flash_suffix = std::to_string(scene_.get_uuid());
    auto muzzle_flash_node_name = "muzzle_flash_node_" + muzzle_flash_suffix;
    scene_.register_node(muzzle_flash_node_name, *muzzle_flash_node);
    node_.add_child(muzzle_flash_node_name, std::move(muzzle_flash_node), ChildRegistrationState::REGISTERED);
    generate_muzzle_flash_hider_(muzzle_flash_suffix);
}

void Gun::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix)
{
    absolute_model_matrix_ = absolute_model_matrix;
}

void Gun::notify_destroyed(Object* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void Gun::trigger(Player* player, Team* team) {
    triggered_ = true;
    player_ = player;
    team_ = team;
}

const TransformationMatrix<float, double, 3>& Gun::absolute_model_matrix() const {
    return absolute_model_matrix_;
}

bool Gun::is_none_gun() const {
    return bullet_lifetime_ == 0;
}

const FixedArray<float, 3>& Gun::punch_angle() const {
    return punch_angle_;
}

float Gun::cool_down() const {
    return cool_down_;
}

float Gun::bullet_damage() const {
    return bullet_damage_;
}
