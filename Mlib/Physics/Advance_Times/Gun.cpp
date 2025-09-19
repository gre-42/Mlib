#include "Gun.hpp"
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
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
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
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
    const DanglingBaseClassRef<SceneNode>& node,
    const DanglingBaseClassPtr<SceneNode>& punch_angle_node,
    const BulletProperties& bullet_properties,
    std::function<void(
        const std::optional<std::string>& player,
        const std::string& bullet_suffix,
        const std::optional<VariableAndHash<std::string>>& target,
        const FixedArray<float, 3>& velocity,
        const FixedArray<float, 3>& angular_velocity)> generate_smart_bullet,
    std::function<void(const AudioSourceState<ScenePos>&)> generate_shot_audio,
    std::function<void(const AudioSourceState<ScenePos>&)> generate_bullet_explosion_audio,
    ITrailStorage* bullet_trace_storage,
    std::string ammo_type,
    std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng,
    std::function<void()> generate_muzzle_flash)
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
    , generate_smart_bullet_{ std::move(generate_smart_bullet) }
    , generate_shot_audio_{ std::move(generate_shot_audio) }
    , generate_bullet_explosion_audio_{ std::move(generate_bullet_explosion_audio) }
    , bullet_trace_storage_{ bullet_trace_storage }
    , ammo_type_{ std::move(ammo_type) }
    , triggered_{ false }
    , player_{ nullptr }
    , team_{ nullptr }
    , cool_down_{ cool_down }
    , time_since_last_shot_{ 0 }
    , absolute_model_matrix_{ fixed_nans<ScenePos, 4, 4 >() }
    , punch_angle_{ 0.f, 0.f, 0.f }
    , punch_angle_rng_{ std::move(punch_angle_rng) }
    , generate_muzzle_flash_{ std::move(generate_muzzle_flash) }
    , node_on_clear_{ node->on_clear, CURRENT_SOURCE_LOCATION }
{
    if (punch_angle_node != nullptr) {
        punch_angle_node_on_clear_.emplace(punch_angle_node->on_clear, CURRENT_SOURCE_LOCATION);
        punch_angle_node_on_clear_->add(
            [this]() { punch_angle_node_ = nullptr; },
            CURRENT_SOURCE_LOCATION);
    }
    node->set_absolute_observer(*this);
    dgs_.add([node]() { if (node->has_absolute_observer()) { node->clear_absolute_observer(); }});
    advance_times_.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    node_on_clear_.add(
        [this]() { node_ = nullptr; global_object_pool.remove(this); },
        CURRENT_SOURCE_LOCATION);
}

Gun::~Gun() {
    on_destroy.clear();
}

void Gun::advance_time(float dt, const StaticWorld& world) {
    time_since_last_shot_ += dt;
    time_since_last_shot_ = std::min(time_since_last_shot_, cool_down_);
    punch_angle_ = punch_angle_rng_(maybe_generate_bullet(world));
    if (punch_angle_node_ != nullptr) {
        punch_angle_node_->set_rotation(punch_angle_, SUCCESSOR_POSE);
    }
    triggered_ = false;
}

size_t Gun::nbullets_available() const {
    return parent_rb_.inventory_.navailable(ammo_type_);
}

bool Gun::maybe_generate_bullet(const StaticWorld& world) {
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
    if (generate_smart_bullet_ &&
        (player_ != nullptr) &&
        !player_->target_id().has_value())
    {
        return false;
    }
    parent_rb_.inventory_.take(ammo_type_, 1);
    time_since_last_shot_ = 0;
    generate_bullet(world);
    if (generate_muzzle_flash_) {
        generate_muzzle_flash();
    }
    if (generate_shot_audio_) {
        generate_shot_audio();
    }
    return true;
}

void Gun::generate_bullet(const StaticWorld& world) {
    auto node = make_unique_scene_node(
        absolute_model_matrix_.t,
        matrix_2_tait_bryan_angles(absolute_model_matrix_.R),
        1.f);
    auto bullet_velocity =
        - bullet_properties_.velocity * z3_from_3x3(absolute_model_matrix_.R)
        + parent_rb_.velocity_at_position(absolute_model_matrix_.t);
    std::string suffix = "_bullet" + scene_.get_temporary_instance_suffix();
    auto bullet_node_name = VariableAndHash<std::string>{"car_node" + suffix};
    if (generate_smart_bullet_) {
        auto np = node.ref(CURRENT_SOURCE_LOCATION);
        scene_.add_root_node(
            bullet_node_name,
            std::move(node),
            RenderingDynamics::MOVING,
            RenderingStrategies::OBJECT);
        generate_smart_bullet_(
            player_ == nullptr ? std::nullopt : std::optional{ player_->id() },
            suffix,
            player_ == nullptr ? std::nullopt : std::optional{ player_->target_id() },
            bullet_velocity,
            parent_rb_.rbp_.w_);
        auto& rc = get_rigid_body_vehicle(np);
        auto bullet = std::make_unique<Bullet>(
            scene_,
            generate_bullet_explosion_audio_,
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
            world,
            RotateBullet::NO);
        // Destruction order: Node -> Rigid body (collision observers) -> Bullet
        // node->clearing_observers.add(*bullet);
        rc.collision_observers_.emplace_back(std::move(bullet));
    } else {
        auto rcu = rigid_cuboid(
            global_object_pool,
            "bullet",
            "bullet_no_id",
            bullet_properties_.mass,
            bullet_properties_.size,
            fixed_zeros<float, 3>(),  // com
            bullet_velocity);
        rcu->flags_ = bullet_properties_.rigid_body_flags;
        auto& rc = *rcu;
        {
            AbsoluteMovableSetter ams{ scene_, node.ref(CURRENT_SOURCE_LOCATION), bullet_node_name, std::move(rcu), CURRENT_SOURCE_LOCATION };
            if (!bullet_properties_.renderable_resource_name->empty()) {
                scene_node_resources_.instantiate_child_renderable(
                    bullet_properties_.renderable_resource_name,
                    ChildInstantiationOptions{
                        .rendering_resources = rendering_resources_,
                        .instance_name = VariableAndHash<std::string>{ "bullet" },
                        .scene_node = node.ref(CURRENT_SOURCE_LOCATION),
                        .interpolation_mode = PoseInterpolationMode::ENABLED,
                        .renderable_resource_filter = RenderableResourceFilter{} });
            }
            rigid_bodies_.add_rigid_body(
                *ams.absolute_movable,
                scene_node_resources_.get_arrays(bullet_properties_.hitbox_resource_name, ColoredVertexArrayFilter{})->scvas,
                scene_node_resources_.get_arrays(bullet_properties_.hitbox_resource_name, ColoredVertexArrayFilter{})->dcvas,
                std::list<TypedMesh<std::shared_ptr<IIntersectable>>>{},
                CollidableMode::MOVING);
            ams.absolute_movable.release();
        }
        auto bullet = std::make_unique<Bullet>(
            scene_,
            generate_bullet_explosion_audio_,
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
            world,
            RotateBullet::YES);
        // Destruction order: Node -> Rigid body (collision observers) -> Bullet
        // node->clearing_observers.add(*bullet);
        rc.collision_observers_.emplace_back(std::move(bullet));
        scene_.add_root_node(
            bullet_node_name,
            std::move(node),
            RenderingDynamics::MOVING,
            RenderingStrategies::OBJECT);
    }
}

void Gun::generate_muzzle_flash() {
    if (!generate_muzzle_flash_) {
        THROW_OR_ABORT("Muzzle flash hider not set");
    }
    generate_muzzle_flash_();
}

void Gun::generate_shot_audio() {
    if (!generate_shot_audio_) {
        THROW_OR_ABORT("Shot audio not set");
    }
    generate_shot_audio_({
        absolute_model_matrix_.t,
        parent_rb_.velocity_at_position(absolute_model_matrix_.t)});
}

void Gun::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix)
{
    absolute_model_matrix_ = absolute_model_matrix;
}

void Gun::trigger(IPlayer* player, ITeam* team) {
    triggered_ = true;
    player_ = player;
    team_ = team;
}

const TransformationMatrix<float, ScenePos, 3>& Gun::absolute_model_matrix() const {
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
