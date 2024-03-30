#include "Gun.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Team/Team.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

Gun::Gun(
    RenderingResources* rendering_resources,
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    SmokeParticleGenerator& smoke_generator,
    DynamicLights& dynamic_lights,
    RigidBodies& rigid_bodies,
    AdvanceTimes& advance_times,
    float cool_down,
    RigidBodyVehicle& parent_rb,
    DanglingRef<SceneNode> node,
    DanglingPtr<SceneNode> punch_angle_node,
    const BulletProperties& bullet_properties,
    ITrailStorage* bullet_trace_storage,
    const std::string& ammo_type,
    const std::function<FixedArray<float, 3>(bool shooting)>& punch_angle_rng,
    const std::string& muzzle_flash_resource,
    const FixedArray<float, 3>& muzzle_flash_position,
    float muzzle_flash_animation_time,
    const std::function<void(const std::string& muzzle_flash_suffix)>& generate_muzzle_flash_hider,
    DeleteNodeMutex& delete_node_mutex)
    : rendering_resources_{ rendering_resources }
    , scene_{ scene }
    , scene_node_resources_{ scene_node_resources }
    , smoke_generator_{ smoke_generator }
    , dynamic_lights_{ dynamic_lights }
    , rigid_bodies_{ rigid_bodies }
    , advance_times_{ advance_times }
    , parent_rb_{ parent_rb }
    , node_{ node.ptr() }
    , punch_angle_node_{ punch_angle_node }
    , bullet_properties_{ bullet_properties }
    , bullet_trace_storage_{ bullet_trace_storage }
    , ammo_type_{ ammo_type }
    , triggered_{ false }
    , player_{ nullptr }
    , team_{ nullptr }
    , cool_down_{ cool_down }
    , time_since_last_shot_{ 0 }
    , absolute_model_matrix_{ fixed_nans<double, 4, 4 >() }
    , punch_angle_{ 0.f, 0.f, 0.f }
    , punch_angle_rng_{ punch_angle_rng }
    , muzzle_flash_resource_{ muzzle_flash_resource }
    , muzzle_flash_position_{ muzzle_flash_position }
    , muzzle_flash_animation_time_{ muzzle_flash_animation_time }
    , generate_muzzle_flash_hider_{ generate_muzzle_flash_hider }
    , delete_node_mutex_{ delete_node_mutex }
    , node_on_clear_{ node->on_clear }
{
    if (punch_angle_node != nullptr) {
        punch_angle_node_on_clear_.emplace(punch_angle_node->on_clear);
        punch_angle_node_on_clear_.value().add([this]() {
            punch_angle_node_ = nullptr;
            });
    }
    node->set_absolute_observer(*this);
    dgs_.add([node]() { if (node->has_absolute_observer()) { node->clear_absolute_observer(); }});
    advance_times_.add_advance_time(*this);
    dgs_.add([this]() { advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION); });
    node_on_clear_.add([this]() {
        node_ = nullptr;
        delete this;
        });
}

Gun::~Gun() = default;

void Gun::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    time_since_last_shot_ += dt;
    time_since_last_shot_ = std::min(time_since_last_shot_, cool_down_);
    punch_angle_ = punch_angle_rng_(maybe_generate_bullet(time));
    if (punch_angle_node_ != nullptr) {
        punch_angle_node_->set_rotation(punch_angle_, SUCCESSOR_POSE);
    }
    triggered_ = false;
}

size_t Gun::nbullets_available() const {
    return parent_rb_.inventory_.navailable(ammo_type_);
}

bool Gun::maybe_generate_bullet(std::chrono::steady_clock::time_point time) {
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
    generate_bullet(time);
    if (!muzzle_flash_resource_.empty()) {
        generate_muzzle_flash_hider();
    }
    return true;
}

void Gun::generate_bullet(std::chrono::steady_clock::time_point time) {
    std::unique_ptr<RigidBodyVehicle> rcu = rigid_cuboid("bullet", "bullet_no_id", bullet_properties_.mass, bullet_properties_.size);
    rcu->flags_ = bullet_properties_.rigid_body_flags;
    rcu->rbp_.v_ =
        - bullet_properties_.velocity * z3_from_3x3(absolute_model_matrix_.R())
        + parent_rb_.rbp_.v_;
    auto node = make_dunique<SceneNode>(
        absolute_model_matrix_.t(),
        matrix_2_tait_bryan_angles(absolute_model_matrix_.R()),
        1.f);
    auto& rc = *rcu;
    {
        AbsoluteMovableSetter ams{node.ref(DP_LOC), std::move(rcu)};
        if (!bullet_properties_.renderable_resource_name.empty()) {
            scene_node_resources_.instantiate_renderable(
                bullet_properties_.renderable_resource_name,
                InstantiationOptions{
                    .rendering_resources = rendering_resources_,
                    .instance_name = "bullet",
                    .scene_node = node.ref(DP_LOC),
                    .renderable_resource_filter = RenderableResourceFilter{}});
        }
        rigid_bodies_.add_rigid_body(
            std::move(ams.absolute_movable),
            scene_node_resources_.get_physics_arrays(bullet_properties_.hitbox_resource_name)->scvas,
            scene_node_resources_.get_physics_arrays(bullet_properties_.hitbox_resource_name)->dcvas,
            CollidableMode::MOVING);
    }
    std::string bullet_node_name = "bullet" + scene_.get_temporary_instance_suffix();
    auto bullet = std::make_unique<Bullet>(
        scene_,
        smoke_generator_,
        advance_times_,
        rc,
        rigid_bodies_,
        player_,
        team_,
        bullet_node_name,
        bullet_properties_,
        bullet_trace_storage_ == nullptr
            ? nullptr
            : bullet_trace_storage_->add_trail_extender(),
        dynamic_lights_,
        delete_node_mutex_,
        time);
    if (player_ != nullptr) {
        player_->destruction_observers.add({ *bullet, CURRENT_SOURCE_LOCATION });
    }
    if (team_ != nullptr) {
        team_->destruction_observers.add({ *bullet, CURRENT_SOURCE_LOCATION });
    }
    advance_times_.add_advance_time(*bullet);
    // Destruction order: Node -> Rigid body (collision observers) -> Bullet
    // node->clearing_observers.add(*bullet);
    rc.collision_observers_.emplace_back(std::move(bullet));
    scene_.add_root_node(bullet_node_name, std::move(node));
}

void Gun::generate_muzzle_flash_hider() {
    std::string muzzle_flash_suffix = smoke_generator_.generate_suffix();

    smoke_generator_.generate_child(
        *node_,
        muzzle_flash_resource_,
        "muzzle_flash_node" + muzzle_flash_suffix,
        muzzle_flash_position_.casted<double>(),
        muzzle_flash_animation_time_);
    generate_muzzle_flash_hider_(muzzle_flash_suffix);
}

void Gun::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix)
{
    absolute_model_matrix_ = absolute_model_matrix;
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
    return bullet_properties_.max_lifetime == 0;
}

const FixedArray<float, 3>& Gun::punch_angle() const {
    return punch_angle_;
}

float Gun::cool_down() const {
    return cool_down_;
}

float Gun::bullet_damage() const {
    return bullet_properties_.damage;
}
